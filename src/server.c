#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "macros.h"

int create_tcp_socket(const int port, const char* bind_addr) {
    struct sockaddr_in server_addr;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, bind_addr, &server_addr.sin_addr.s_addr);

    int on = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        ERR("error in bind: %s\n", strerror(errno));
        return 1;
    }

    return socket_fd;
}
