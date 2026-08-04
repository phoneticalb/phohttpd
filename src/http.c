#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include <fcntl.h>
#include <unistd.h>

#define CHUNK_SIZE 512

static char* normalize_path(char* path) {
    static char new_path[256];
    char cwd[256] = { 0 };
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        ERR("error in getcwd: %s\n", strerror(errno));
    }

    snprintf(new_path, sizeof(new_path), "%s%s", cwd, path);
    return new_path;
}

int http_req(char* data, int fd) {
    char** headers;
    char* token;

    char* buf = strdup(data);

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

    DEBUG("requested path: %s\n", normalize_path(path));

    FILE* file = fopen(normalize_path(path), "r");
    if (file == NULL) {
        ERR("error in fopen: %s\n", strerror(errno));
        // switch (errno) {
        //     case ENOENT:
        //         snprintf(response, 256, "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
        //         break;
        //     case EACCES:
        //         snprintf(response, 256, "HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n");
        //         break;
        //     default:
        //         snprintf(response, 256, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        //         break;
        //     }
    }

    char* http_ok = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";
    ssize_t http_ok_len = strlen(http_ok);

    write(fd, http_ok, http_ok_len);

    char chunk[CHUNK_SIZE];
    ssize_t nbytes = 0;
    while ((nbytes = fread(chunk, sizeof(char), CHUNK_SIZE, file))) {
        if (write(fd, chunk, nbytes) == -1) {
            ERR("didn't write to socket: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}
