#include <bits/posix1_lim.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "server.h"
#include <netinet/in.h>
#include "macros.h"
#include "http.h"

int main(int argc, char* argv[]) {
    INFO(ANSI_BOLD "phohttpd " ANSI_RESET "%s\n", VERSION);
    struct sockaddr_in client_addr;
    const int port = 8080;
    const char* bind_addr = "127.0.0.1";
    socklen_t client_addr_len = sizeof(client_addr);

    int tcp_fd = create_tcp_socket(port, bind_addr);

    listen(tcp_fd, 5);

    INFO("listening on %s:%d\n", bind_addr, port);

    while (1) {
        int conn_fd = accept(tcp_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        if (!fork()) {
            close(tcp_fd);

            // for now
            char buffer[1024] = {};

            read(conn_fd, buffer, sizeof(buffer));

            if (http_req(buffer, conn_fd) == -1) {
                ERR("error processing HTTP req\n");
                exit(1);
            }

            close(conn_fd);
            exit(0);
        }

        close(conn_fd);
    }

    close(tcp_fd);

    return 0;
}
