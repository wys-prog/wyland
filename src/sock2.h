#ifndef ___WYLAND_SOCK_H___
#define ___WYLAND_SOCK_H___

#ifdef _WIN32
#include <windows.h>
#endif 

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
           ((val & 0x000000FF00000000) >> 8)  |
           ((val & 0x00000000FF000000) << 8)  |
           ((val & 0x0000000000FF0000) << 24) |
           ((val & 0x000000000000FF00) << 40) |
           (val >> 56);
}

#define SWAP16(x) ((uint16_t)((((x) & 0x00FF) << 8) | \
                              (((x) & 0xFF00) >> 8)))

#define SWAP32(x) ((uint32_t)((((x) & 0x000000FF) << 24) | \
                              (((x) & 0x0000FF00) << 8)  | \
                              (((x) & 0x00FF0000) >> 8)  | \
                              (((x) & 0xFF000000) >> 24)))

#define SWAP64(x) __builtin_bswap64(x)

#define SWAP128(x) __wyland_swap128(x)

inline __uint128_t __wyland_swap128(__uint128_t x) {
    uint64_t lo = (uint64_t)x;
    uint64_t hi = (uint64_t)(x >> 64);
    lo = __builtin_bswap64(lo);
    hi = __builtin_bswap64(hi);
    return ((__uint128_t)lo << 64) | hi;
}

template <typename T>
inline T swap(const T &__t) {
    std::cout << "little: " << __t << std::endl;
    T result;
    const char *src = reinterpret_cast<const char *>(&__t);
    char *dest = reinterpret_cast<char *>(&result);
    for (size_t i = 0; i < sizeof(T); ++i) {
        dest[i] = src[sizeof(T) - 1 - i];
    }
    std::cout << "big: " << result << std::endl;
    return result;
}

template <typename T>
inline T correct_byte_order(const T &__t) {
#if IS_LITTLE_ENDIAN
    return swap<T>(__t);
#else
    return __t;
#endif
}

// Detect endianness at compile-time
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define IS_BIG_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define IS_LITTLE_ENDIAN 1
#elif defined(_WIN32)
    // Windows is always little-endian
    #define IS_LITTLE_ENDIAN 1
#elif defined(__APPLE__) && defined(__MACH__)
    // macOS is always little-endian (Intel and ARM)
    #define IS_LITTLE_ENDIAN 1
#else
    #error "Unable to determine endianness for this platform"
#endif

#if IS_BIG_ENDIAN
#define correct_byte_order_8(x)              x
#define correct_byte_order_16(x)             x
#define correct_byte_order_32(x)             x
#define correct_byte_order_64(x)             x
#define correct_byte_order_128(x)            x
#else
#define correct_byte_order_8(x)              (x)
#define correct_byte_order_16(x)             (SWAP16(x))
#define correct_byte_order_32(x)             (SWAP32(x))
#define correct_byte_order_64(x)             (SWAP64(x))
#define correct_byte_order_128(x)            (SWAP128(x))
#endif // ? BIG ENDIAN ?

#endif