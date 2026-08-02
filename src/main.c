#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include <netinet/in.h>

int main(int argc, char* argv[]) {
    struct sockaddr_in client_sockaddr_in;
    const int port = 8080;
    const char* bind_addr = "0.0.0.0";
    socklen_t len = sizeof(client_sockaddr_in);

    int tcp_fd = create_tcp_socket(port, bind_addr);

    listen(tcp_fd, 5);

    fprintf(stderr, "Listening on %s:%d\n", bind_addr, port);

    while (1) {
        printf("new\n");
        int conn_fd = accept(tcp_fd, (struct sockaddr*)&client_sockaddr_in, &len);

        char buffer[256] = {};
        read(conn_fd, buffer, sizeof(buffer));

        printf("%s", buffer);

        char status = 0;
        write(conn_fd, &status, 1);
    }

    close(tcp_fd);

    return 0;
}
