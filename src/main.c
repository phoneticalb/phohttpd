#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include <netinet/in.h>
#include "macros.h"
#include "http.h"

int main(int argc, char* argv[]) {
    struct sockaddr_in client_addr;
    const int port = 8080;
    const char* bind_addr = "0.0.0.0";
    socklen_t client_addr_len = sizeof(client_addr);

    int tcp_fd = create_tcp_socket(port, bind_addr);

    listen(tcp_fd, 5);

    INFO("listening on %s:%d\n", bind_addr, port);

    while (1) {
        int conn_fd = accept(tcp_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        if (!fork()) {
            close(tcp_fd);

            char buffer[1024] = {};
            read(conn_fd, buffer, sizeof(buffer));

            char* response = http_req(buffer);

            write(conn_fd, response, strlen(response));
            close(conn_fd);
            exit(0);
        }

        close(conn_fd);
    }

    close(tcp_fd);

    return 0;
}
