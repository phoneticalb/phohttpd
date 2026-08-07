#include <sys/types.h>

int is_dir(const char* path);

// returns 0 on success, -1 on failure
int http_process_req(char* data, int sockfd, char* dir);
