#ifndef ___WYLAND_TYPES_H___
#define ___WYLAND_TYPES_H___

#ifdef _WIN32
#include <windows.h>
#endif 

#include <stdint.h>

typedef void (*lambda)();

typedef __int8_t              wi8;
typedef __int16_t             wi16;
typedef __int32_t             wi32;
typedef __int64_t             wi64;
typedef __int128_t            wi128;
typedef __uint8_t             wui8;
typedef __uint16_t            wui16;
typedef __uint32_t            wui32;
typedef __uint64_t            wui64;
typedef __uint128_t           wui128;

#endif // ___WYLAND_TYPES_H___