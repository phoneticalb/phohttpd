#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int create_tcp_socket(const int port, const char* bind_addr) {
    struct sockaddr_in server_sockaddr_in;
    struct sockaddr_in client_sockaddr_in;
    socklen_t len = sizeof(client_sockaddr_in);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_port = htons(port);
    inet_pton(AF_INET, bind_addr, &server_sockaddr_in.sin_addr.s_addr);

    if (bind(socket_fd, (struct sockaddr *)&server_sockaddr_in, sizeof(server_sockaddr_in)) == -1) {
        printf("bind: %s\n", strerror(errno));
        return 1;
    }

    listen(socket_fd, 5);

    printf("Listening on %s:%d\n", bind_addr, port);

    int conn_fd = accept(socket_fd, (struct sockaddr *)&client_sockaddr_in, &len);

    char buffer[256] = {};
    read(conn_fd, buffer, sizeof(buffer));
    printf("%s", buffer);

    char status = 0;
    write(conn_fd, &status, 1);

    return socket_fd;
}
