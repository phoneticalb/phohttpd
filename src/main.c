#include "http.h"
#include "macros.h"
#include "server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static const struct option long_options[] = {
    {"dir", required_argument, NULL, 'd'},    {"port", required_argument, NULL, 'p'},
    {"listen", required_argument, NULL, 'l'}, {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},      {0, 0, 0}};

static const char usage[] = "Usage: phohttpd [options]\n"
                            "\n"
                            "  -d  --dir <path>        Directory to serve files from\n"
                            "  -p  --port <port>       Port to listen on (default: 8080)\n"
                            "  -l  --listen <addr>     Address to listen on (default: 0.0.0.0)\n"
                            "  -h, --help              Show this menu\n"
                            "  -v, --version           Print version number\n"
                            "\n";

static void format_request(struct HttpRequest req, char* dest) {
    char* method;
    char* version;
    char* status;

    switch (req.method) {
    case GET:
        method = "GET";
        break;
    case HEAD:
        method = "HEAD";
        break;
    default:
        method = "???";
        break;
    }

    switch (req.version) {
    case HTTP_INVALID:
        version = "invalid";
        break;
    case HTTP_10:
        version = "HTTP/1.0";
        break;
    case HTTP_11:
        version = "HTTP/1.1";
        break;
    default:
        version = "???";
        break;
    }

    switch (req.status) {
    case HTTP_OK:
        status = "200 OK";
        break;
    case HTTP_BAD_REQUEST:
        status = "400 Bad Request";
        break;
    case HTTP_FORBIDDEN:
        status = "403 Forbidden";
        break;
    case HTTP_NOT_FOUND:
        status = "404 Not Found";
        break;
    case HTTP_INTERNAL_SERVER_ERROR:
        status = "500 Internal Server Error";
        break;
    case HTTP_VERSION_NOT_SUPPORTED:
        status = "505 HTTP Version Not Supported";
        break;
    default:
        status = "??? Unknown";
        break;
    }

    ssize_t len = strlen(method) + strlen(req.path) + strlen(version) + strlen(status) + 4;

    snprintf(dest, len, "%s %s %s %s", method, req.path, version, status);
}

int main(int argc, char* argv[]) {
    INFO(ANSI_BOLD "phohttpd " ANSI_RESET "%s\n", VERSION);
    struct sockaddr_in client_addr;
    socklen_t          client_addr_len = sizeof(client_addr);

    unsigned int port = 8080;
    char*        bind_addr = "0.0.0.0";
    char*        directory = NULL;
    char         c;
    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "d:p:l:hv", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'd': // dir
            directory = optarg;
            break;
        case 'p':
            port = strtoul(optarg, NULL, 10);
            break;
        case 'l':
            bind_addr = optarg;
            break;
        case 'h': // help
            INFO("%s", usage);
            return 1;
        case 'v': // version
            return 1;
        default:
            INFO("%s", usage);
            return 1;
        }
    }

    if (directory) {
        if (is_dir(directory)) {
            INFO("serving directory: %s\n", directory);
        } else {
            ERR("invalid directory: %s\n", directory);
            return 1;
        }
    } else {
        INFO("serving current directory\n");
    }

    const int tcp_fd = create_tcp_socket(port, bind_addr);
    if (tcp_fd == -1)
        return 1;

    listen(tcp_fd, 128);

    struct sigaction act = {0};

    act.sa_handler = SIG_IGN;

    sigaction(SIGCHLD, &act, NULL);

    INFO("listening on %s:%d\n", bind_addr, port);

    while (1) {
        int conn_fd = accept(tcp_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        if (!fork()) {
            close(tcp_fd);

            const int req_limit = 4096;
            char      buffer[req_limit];

            if (read(conn_fd, buffer, req_limit) == -1 && errno != EAGAIN) {
                WARN("error reading from socket: %s\n", strerror(errno));
                close(conn_fd);
                exit(EXIT_FAILURE);
            };

            struct HttpRequest req = http_process_req(buffer, conn_fd, directory);

            char client_ip[16];
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, 16);

            char req_str[256];
            format_request(req, req_str);

            INFO("request from %s:%d | %s\n", client_ip, client_addr.sin_port, req_str);

            close(conn_fd);
            exit(EXIT_SUCCESS);
        }

        close(conn_fd);
    }

    close(tcp_fd);

    return 0;
}
