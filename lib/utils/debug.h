#pragma once
#include <cstdio>

#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("[DEBUG] " __VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif
