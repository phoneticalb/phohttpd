#include <unistd.h>
#include "server.h"

int main(int argc, char* argv[]) {
    int tcp_fd = create_tcp_socket(8080, "0.0.0.0");

    close(tcp_fd);

    return 0;
}
