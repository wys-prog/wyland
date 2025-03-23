#ifndef ___WYLAND_SOCK_H___
#define ___WYLAND_SOCK_H___

#include <stdint.h>

// Swap functions
uint16_t swap16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

uint32_t swap32(uint32_t val) {
    return (val << 24) |
           ((val & 0x00FF0000) >> 8) |
           ((val & 0x0000FF00) << 8) |
           (val >> 24);
}

uint64_t swap64(uint64_t val) {
    return (val << 56) |
           ((val & 0x00FF000000000000) >> 40) |
           ((val & 0x0000FF0000000000) >> 24) |
           ((val & 0x000000FF00000000) >> 8) |
           ((val & 0x00000000FF000000) << 8) |
           ((val & 0x0000000000FF0000) << 24) |
           ((val & 0x000000000000FF00) << 40) |
           (val >> 56);
}


#endif