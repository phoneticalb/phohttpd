#include <errno.h>
#include <string.h>
#include "macros.h"
#include <fcntl.h>
#include <unistd.h>

static char* normalize_path(char* path) {
    static char new_path[256];
    char cwd[256] = { 0 };
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        ERR("error in getcwd: %s\n", strerror(errno));
    }

    snprintf(new_path, sizeof(new_path), "%s%s", cwd, path);
    return new_path;
}

char* http_req(char* data) {
    static char response[4096];
    char** headers;
    char* token;
    char* buf = strdup(data);
    char file_buf[4096] = { 0 };

    token = strtok(buf, " ");
    char* method = strdup(token);

    token = strtok(NULL, " ");
    char* path = strdup(token);

    token = strtok(NULL, " \r\n");
    char* version = strdup(token);

    // header parsing TODO FIX IDK WHATS GOING ON
    // int header_amt = 0;
    // char* header_buf = strdup(data);
    // token = strtok(header_buf, "\r\n");

    // while ((token = strtok(NULL, "\r\n")) != NULL) {
    //     headers[header_amt] = token;
    //     header_amt++;
    // }

    // iterate headers
    // for (int i = 0; i < header_amt; i++) {
    //     INFO("%s\n", headers[i]);
    // }

    INFO("path: %s\n", normalize_path(path));

    // FILE* file = fopen(normalize_path(path), "r");

    int file = open(normalize_path(path), O_RDONLY);
    if (file == -1) {
        ERR("error in open: %s\n", strerror(errno));
        switch (errno) {
            case ENOENT:
                snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
                break;
            case EACCES:
                snprintf(response, sizeof(response), "HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n");
                break;
            default:
                snprintf(response, sizeof(response), "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
                break;
            }

        return response;
    }

    if (read(file, &file_buf, sizeof(file_buf)) == -1) {
        ERR("error in read: %s\n", strerror(errno));
        snprintf(response, sizeof(response), "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        return response;
    }

    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n%s", file_buf);

    close(file);
    return response;
}
