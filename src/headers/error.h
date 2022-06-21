#pragma once
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define DEBUG 0
#define handle_error(msg) ({\
    perror(msg);\
    exit(EXIT_FAILURE);\
})