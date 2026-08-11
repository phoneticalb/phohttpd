#include "err_pages.h"
#include "macros.h"
#include "http.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
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

static char* fix_path(char* path, const char* dir) {
    static char new_path[256];

    // remove ..
    char* nodots = str_remove(path, "/..");

    if (dir != NULL) {
        snprintf(new_path, sizeof(new_path), "%s%s", dir, nodots);
    } else
        snprintf(new_path, sizeof(new_path), ".%s", nodots);

    // remove ? queries for now
    char* qmark;
    qmark = strchr(new_path, '?');
    if (qmark != NULL) {
        *qmark = '\0';
    }

    // remove trailing slashes
    char* slash = &new_path[strlen(new_path) - 1];
    if (slash && *slash == '/')
        *slash = '\0';

    // parse ascii url encodings
    char* percent;
    while ((percent = strchr(new_path, '%')) != NULL) {
        int ascii_hex;
        sscanf(percent, "%%%x", &ascii_hex);
        if (ascii_hex < 32 || ascii_hex > 126)
            break;

        char dec = (char)ascii_hex;
        for (int i = 0; i < 3; i++)
            percent[i] = '\0';

        ssize_t nbytes = 0;
        for (int i = 3; percent[i] != '\0'; i++)
            nbytes++;

        memcpy(percent + 1, percent + 3, nbytes);
        memset(percent + nbytes + 1, 0, 8);

        percent[0] = dec;
    }

    return new_path;
}

// write file to socket
static void write_file(int fd, FILE* file) {
    char    chunk[CHUNK_SIZE];
    ssize_t nbytes = 0;
    while ((nbytes = fread(chunk, sizeof(char), CHUNK_SIZE, file))) {
        if (write(fd, chunk, nbytes) == -1) {
            WARN("error writing to socket: %s\n", strerror(errno));
            return;
        }
    }
    return;
}

static long get_file_size(FILE* file) {
    long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

static void http_send_resp(struct HttpRequest req, const int sockfd, FILE* file) {
    char*   status_line = NULL;
    char    default_headers[] = "\r\nServer: phohttpd\r\nConnection: close\r\n\r\n\n";
    char    time_str[256];
    char    cl_str[256];
    ssize_t resp_len;
    long    file_size;

    if (file != NULL) {
        file_size = get_file_size(file);
        snprintf(cl_str, sizeof(cl_str), "\r\nContent-Length: %ld", file_size);
    }

    time_t     t = time(NULL);
    struct tm* time_tmp = localtime(&t);

    if (time_tmp == NULL) {
        WARN("error in localtime: %s\n", strerror(errno));
        snprintf(time_str, sizeof(time_str), "\r\nDate: unknown");
    } else
        strftime(time_str, sizeof(time_str), "\r\nDate: %a, %d %b %Y %T %Z", time_tmp);

    ssize_t headers_len = strlen(default_headers) + sizeof(time_str) + sizeof(cl_str);
    char    headers[headers_len];

    snprintf(headers, headers_len, "%s%s%s", time_str, cl_str, default_headers);

    switch (req.status) {
    case HTTP_OK:
        status_line = "HTTP/1.1 200 OK";
        break;
    case HTTP_BAD_REQUEST:
        status_line = "HTTP/1.1 400 Bad Request";
        break;
    case HTTP_FORBIDDEN:
        status_line = "HTTP/1.1 403 Forbidden";
        break;
    case HTTP_NOT_FOUND:
        status_line = "HTTP/1.1 404 Not Found";
        break;
    case HTTP_INTERNAL_SERVER_ERROR:
        status_line = "HTTP/1.1 500 Internal Server Error";
        break;
    case HTTP_VERSION_NOT_SUPPORTED:
        status_line = "HTTP/1.1 505 HTTP Version Not Supported";
        break;
    default:
        WARN("unimplemented status code: %d\n", req.status);
        break;
    }

    if (status_line != NULL) {
        resp_len = strlen(status_line) + strlen(headers);
        char response_str[resp_len];
        snprintf(response_str, resp_len, "%s%s", status_line, headers);
        write(sockfd, response_str, resp_len - 1);
    }

    if (file != NULL)
        write_file(sockfd, file);
}

int is_dir(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

struct HttpRequest http_process_req(char* data, const int sockfd, const char* dir) {
    struct HttpRequest req;

    char method_str[8];
    char version_str[16];

    char* token;
    char* save;

    if ((token = strtok_r(data, " ", &save)) != NULL)
        strncpy(method_str, token, sizeof(method_str));

    if ((token = strtok_r(NULL, " ", &save)) != NULL)
        strncpy(req.path, token, sizeof(req.path));

    if ((token = strtok_r(NULL, " \r\n", &save)) != NULL)
        strncpy(version_str, token, sizeof(version_str));

    if (method_str[0] == '\0' || req.path[0] == '\0' || version_str[0] == '\0') {
        req.status = HTTP_BAD_REQUEST;
        http_send_resp(req, sockfd, NULL);
        write(sockfd, error_400_page, strlen(error_400_page));
        return req;
    }

    if (!strncmp(method_str, "GET", 4))
        req.method = GET;
    else if (!strncmp(method_str, "HEAD", 5))
        req.method = HEAD;
    else {
        req.status = HTTP_BAD_REQUEST;
        http_send_resp(req, sockfd, NULL);
        write(sockfd, error_400_page, strlen(error_400_page));
        return req;
    }

    if (!strncmp(version_str, "HTTP/1.0", 9))
        req.version = HTTP_10;
    else if (!strncmp(version_str, "HTTP/1.1", 9))
        req.version = HTTP_11;
    else
        req.version = HTTP_INVALID;

    if (req.version == HTTP_INVALID) {
        req.status = HTTP_VERSION_NOT_SUPPORTED;
        http_send_resp(req, sockfd, NULL);
        write(sockfd, error_505_page, strlen(error_505_page));
        return req;
    }

    if (req.method == HEAD) {
        req.status = HTTP_OK;
        http_send_resp(req, sockfd, NULL);
        return req;
    }

    char  file_path[256];
    char* tmp_path = fix_path(req.path, dir);

    if (is_dir(tmp_path))
        snprintf(file_path, strlen(tmp_path) + 12, "%s/index.html", tmp_path);
    else
        snprintf(file_path, strlen(tmp_path) + 1, "%s", tmp_path);

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        switch (errno) {
        case ENOENT:
            req.status = HTTP_NOT_FOUND;
            http_send_resp(req, sockfd, NULL);
            write(sockfd, error_404_page, strlen(error_404_page));
            return req;
        case EACCES:
            req.status = HTTP_FORBIDDEN;
            http_send_resp(req, sockfd, NULL);
            write(sockfd, error_403_page, strlen(error_403_page));
            return req;
        default:
            req.status = HTTP_INTERNAL_SERVER_ERROR;
            http_send_resp(req, sockfd, NULL);
            write(sockfd, error_500_page, strlen(error_500_page));
            return req;
        }
    }

    if (req.method == GET) {
        req.status = HTTP_OK;
        http_send_resp(req, sockfd, file);
    }

    fclose(file);

    return req;
}
