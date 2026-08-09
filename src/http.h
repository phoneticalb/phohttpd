#include <sys/types.h>

int is_dir(const char* path);

// returns 0 on success, -1 on failure
struct HttpRequest http_process_req(char* data, const int sockfd, const char* dir);

enum HttpMethod {
    GET = 0,
    HEAD = 1,
};

enum HttpVersion {
    HTTP_INVALID = -1,
    HTTP_10 = 0,
    HTTP_11 = 1,
};

enum HttpStatus {
    HTTP_OK = 200,
    HTTP_BAD_REQUEST = 400,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_VERSION_NOT_SUPPORTED = 505,
};

struct HttpRequest {
    enum HttpMethod method;
    enum HttpVersion version;
    enum HttpStatus status;
    char path[256];
};
