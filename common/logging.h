#pragma once
#include <stdio.h>

#define ERROR(fmt, ...) fprintf(stderr, "error: " fmt "\n", ##__VA_ARGS__)

#ifdef DEBUG
    #define DBG(fmt, ...) fprintf(stderr, "debug: " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG(fmt, ...)
#endif
