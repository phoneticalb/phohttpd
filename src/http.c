#include "err_pages.h"
#include "macros.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHUNK_SIZE 512

// https://stackoverflow.com/a/47117431
static char* str_remove(char* str, const char* sub) {
    char *p, *q, *r;
    if (*sub && (q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            memmove(q, p, r - p);
            q += r - p;
        }
        memmove(q, p, strlen(p) + 1);
    }
    return str;
}

static char* fix_path(char* path, char* dir) {
    static char new_path[256];

    char* nodots = str_remove(path, "/..");

    if (dir != NULL) {
        snprintf(new_path, sizeof(new_path), "./%s%s", dir, nodots);
    } else
        snprintf(new_path, sizeof(new_path), ".%s", nodots);

    // for now
    char* qmark;
    qmark = strchr(new_path, '?');
    if (qmark != NULL) {
        *qmark = '\0';
    }

    char* slash = &new_path[strlen(new_path) - 1];
    if (slash && *slash == "/"[0])
        *slash = '\0';

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
    char*   response_str;
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

int is_dir(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

int http_req(char* data, int sockfd, char* dir) {
    char* token;
    char* save;

    char method[8];
    char path[256];
    char version[16];

    token = strtok_r(data, " ", &save);
    strncpy(method, token, sizeof(method));

    token = strtok_r(NULL, " ", &save);
    strncpy(path, token, sizeof(path));

    token = strtok_r(NULL, " \r\n", &save);
    strncpy(version, token, sizeof(version));

    if (strncmp(method, "GET", 4)) {
        http_resp(400, sockfd, NULL);
        write(sockfd, error_400_page, strlen(error_400_page));
        return 1;
    }

    char  req_path[256];
    char* tmp_path = fix_path(path, dir);

    if (is_dir(tmp_path))
        snprintf(req_path, strlen(tmp_path) + 12, "%s/index.html", tmp_path);
    else
        snprintf(req_path, strlen(tmp_path) + 1, "%s", tmp_path);

    DEBUG("requested path: %s\n", req_path);

    FILE* req_file = fopen(req_path, "r");
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
