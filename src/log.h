#ifndef C_TRIS_LOG_H_
#define C_TRIS_LOG_H_

#include <stdio.h>

#ifdef DEBUG
#define LOG_LEVEL 1
#else
#define LOG_LEVEL 0
#endif

#define LOG_INFO(fmt, ...)                                                     \
    do {                                                                       \
        if (LOG_LEVEL)                                                         \
            fprintf(stdout, fmt, __VA_ARGS__);                                 \
    } while (0)

#define LOG_ERROR(fmt, ...)                                                    \
    do {                                                                       \
        if (LOG_LEVEL)                                                         \
            fprintf(stderr, fmt, __VA_ARGS__);                                 \
    } while (0)

#endif
