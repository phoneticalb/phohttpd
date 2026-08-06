#include "http.h"
#include "macros.h"
#include "server.h"
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

static const struct option long_options[] = {{"dir", required_argument, NULL, 'd'},
                                             {"help", no_argument, NULL, 'h'},
                                             {"version", no_argument, NULL, 'v'},
                                             {0, 0, 0}};

static const char usage[] = "Usage: phohttpd [options]\n"
                            "\n"
                            "  -d  --dir <path>        Directory to serve files from\n"
                            "  -h, --help              Show this menu\n"
                            "  -v, --version           Print version number\n"
                            "\n";

int main(int argc, char* argv[]) {
    INFO(ANSI_BOLD "phohttpd " ANSI_RESET "%s\n", VERSION);
    struct sockaddr_in client_addr;
    const int          port = 8080;
    const char*        bind_addr = "127.0.0.1";
    socklen_t          client_addr_len = sizeof(client_addr);

    int   c;
    char* directory = NULL;
    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "d:hv", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'd': // dir
            directory = optarg;
            break;
        case 'h': // help
            INFO("%s", usage);
            exit(EXIT_SUCCESS);
            break;
        case 'v': // version
            exit(EXIT_SUCCESS);
            break;
        default:
            INFO("%s", usage);
            exit(EXIT_FAILURE);
            break;
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

    int tcp_fd = create_tcp_socket(port, bind_addr);
    if (tcp_fd == -1)
        return 1;

    listen(tcp_fd, 64);

    signal(SIGPIPE, SIG_IGN);

    INFO("listening on %s:%d\n", bind_addr, port);

    while (1) {
        int conn_fd = accept(tcp_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        int wstatus;
        pid_t cpid, w;

        cpid = fork();
        if (cpid == 0) { // child runs this block
            close(tcp_fd);

            // for now
            char buffer[1024] = {};

            if (read(conn_fd, buffer, sizeof(buffer)) == -1) {
                WARN("error reading from socket: %s\n", strerror(errno));
                close(conn_fd);
                exit(EXIT_FAILURE);
            };

            if (http_req(buffer, conn_fd, directory) == -1) {
                WARN("invalid http request\n");
                close(conn_fd);
                exit(EXIT_FAILURE);
            }

            close(conn_fd);
            exit(EXIT_SUCCESS);
        } else { // parent runs this block
            do {
                w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
                if (w == -1) {
                    ERR("error in waitpid: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (WIFSIGNALED(wstatus)) {
                    int signal = WTERMSIG(wstatus);
                    ERR("child was killed: %s\n", strsignal(signal));
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
        }

        close(conn_fd);
    }

    close(tcp_fd);

    return 0;
}
