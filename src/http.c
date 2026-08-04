#include "macros.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err_pages.h"

#define CHUNK_SIZE 512

static char* normalize_path(char* path) {
    static char new_path[256];
    char        cwd[256] = {0};
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        ERR("error in getcwd: %s\n", strerror(errno));
    }

    snprintf(new_path, sizeof(new_path), "%s%s", cwd, path);
    return new_path;
}

// write file to socket for http response
static void sock_write(int fd, FILE* file) {
    char    chunk[CHUNK_SIZE];
    ssize_t nbytes = 0;
    while ((nbytes = fread(chunk, sizeof(char), CHUNK_SIZE, file))) {
        if (write(fd, chunk, nbytes) == -1) {
            ERR("couldn't write to socket: %s\n", strerror(errno));
            return;
        }
    }
    return;
}

static void http_resp(int status, int sockfd, FILE* file) {
    char* response_str;
    ssize_t len;
    switch (status) {
        case 200:
            response_str = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";
            break;
        case 400:
            response_str = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
            break;
        case 403:
            response_str = "HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n";
            break;
        case 404:
            response_str = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
            break;
        case 500:
            response_str = "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
            break;
        default:
            WARN("unimplemented status code: %d\n", status);
            break;
    }
    if (response_str != NULL) {
        len = strlen(response_str);
        write(sockfd, response_str, len);
    }
    if (file != NULL)
        sock_write(sockfd, file);
}

int http_req(char* data, int sockfd) {
    char* token;
    char* buf = strdup(data);

    token = strtok(buf, " ");
    char* method = strdup(token);

    token = strtok(NULL, " ");
    char* path = strdup(token);

    token = strtok(NULL, " \r\n");
    char* version = strdup(token);

    DEBUG("requested path: %s\n", normalize_path(path));

    FILE* req_file = fopen(normalize_path(path), "r");
    if (req_file == NULL) {
        ERR("error in fopen: %s\n", strerror(errno));
        switch (errno) {
            case ENOENT:
                http_resp(404, sockfd, NULL);
                write(sockfd, error_404_page, strlen(error_404_page));
                return 1;
            case EACCES:
                http_resp(403, sockfd, NULL);
                write(sockfd, error_403_page, strlen(error_403_page));
                return 1;
            default:
                http_resp(500, sockfd, NULL);
                write(sockfd, error_500_page, strlen(error_500_page));
                return 1;
            }
    }

    http_resp(200, sockfd, req_file);

    fclose(req_file);

    return 0;
}
