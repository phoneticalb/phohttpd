#include <stdio.h>

#define ANSI_RED "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_PURPLE "\x1b[35m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_RESET "\x1b[0m"

#define INFO(...)                                                                                  \
    do {                                                                                           \
        fprintf(stderr, "[" ANSI_BLUE "i" ANSI_RESET "] ");                                        \
        fprintf(stderr, __VA_ARGS__);                                                              \
    } while (0)

#define WARN(...)                                                                                  \
    do {                                                                                           \
        fprintf(stderr, "[" ANSI_YELLOW "w" ANSI_RESET "] ");                                      \
        fprintf(stderr, __VA_ARGS__);                                                              \
    } while (0)

#define DEBUG(...)                                                                                 \
    do {                                                                                           \
        fprintf(stderr, "[" ANSI_PURPLE "d" ANSI_RESET "] ");                                      \
        fprintf(stderr, __VA_ARGS__);                                                              \
    } while (0)

#define ERR(...)                                                                                   \
    do {                                                                                           \
        fprintf(stderr, "[" ANSI_RED "e" ANSI_RESET "] ");                                         \
        fprintf(stderr, __VA_ARGS__);                                                              \
    } while (0)
