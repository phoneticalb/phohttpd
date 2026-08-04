#include <errno.h>
#include <stdlib.h>
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

ssize_t http_req(char* data, char** response) {
    unsigned char* file_buf = NULL;
    ssize_t file_buf_size = 0;
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

    INFO("path: %s\n", normalize_path(path));

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

    if (file) {
        fseek(file, 0, SEEK_END);
        file_buf_size = ftell(file);
        rewind(file);

        file_buf = malloc(file_buf_size);
        if (file_buf == NULL) {
            ERR("error when malloc'ing file: %s\n", strerror(errno));
            return -1;
        }

        fread(file_buf, 1, file_buf_size, file);

        fclose(file);
    }

    char* http_ok = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";
    ssize_t http_ok_len = strlen(http_ok);
    ssize_t response_len = http_ok_len + file_buf_size;

    char* temp = realloc(*response, response_len);
    if (temp == NULL) {
        ERR("error in realloc: %s\n", strerror(errno));
        free(file_buf);
        return -1;
    } else {
        *response = temp;
    }

    memcpy(*response, http_ok, http_ok_len);
    memcpy(*response + http_ok_len, file_buf, file_buf_size);

    free(file_buf);

    return response_len;
}
