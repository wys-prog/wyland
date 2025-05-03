#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include <vector>
#include <cstdint>
#include <stdexcept>

#define WYLAND_BEGIN namespace wylma { namespace wyland { 
#define WYLAND_END   } }

#define SYSTEM_SEGMENT_SIZE 12_MB

#define SYSTEM_SEGMENT_START 0
#define HARDWARE_SEGMENT_START (SYSTEM_SEGMENT_START + SYSTEM_SEGMENT_SIZE)

#define KEYBOARD_SEGMENT_START HARDWARE_SEGMENT_START
#define KEYBOARD_SEGMENT_END   HARDWARE_SEGMENT_START + 2_MB 

#define WYLAND_SYSTEM_INT_BEG 10_MB

#define WYLAND_MEMORY_MINIMUM 16_MB

#define REG_KEY (32)
#define R_RELATIVE_N 59
#define R_MEMORY_INF 60
#define R_POP_MMIO 61
#define R_ORG 62
#define R_RET 63
#define R_STACK_BASE 64
#define STACK_SIZE (1024*4)

#define EQUAL  (0)
#define LARGER (1)
#define LESSER (2)

#ifdef _WIN32
#define STRDUP(x) _strdup(x)
#else 
#define STRDUP(x) strdup(x)
#endif // Is Windows ?

#define mnameof(x) #x

constexpr std::size_t operator""_MB(unsigned long long size) {
  return size * 1024 * 1024;
}

constexpr std::size_t operator""_GB(unsigned long long size) {
  return size * 1024 * 1024 * 1024;
}

constexpr std::size_t operator""_KB(unsigned long long size) {
  return size * 1024;
}

constexpr std::size_t operator""_B(unsigned long long size) {
  return size;
}

WYLAND_BEGIN

uint8_t *memory;
uint64_t code_start;

namespace segments {
  static bool keyboard_reserved;
  static uint64_t memory_size = WYLAND_MEMORY_MINIMUM;
}

namespace manager {
  /* Don't use this namespace ! */
  namespace _ {
    std::vector<std::pair<uint64_t, uint64_t>> regions;
    bool regions_reserved;
  }

  void create_region(uint64_t a, uint64_t b) {
    while (_::regions_reserved) ;
    _::regions_reserved = true;

    for (const auto &pair : _::regions) {
      if (!(b <= pair.first || a >= pair.second)) 
        throw std::logic_error("Thread memory region overlaps with an existing one.");
    }

    _::regions.push_back({a, b});

    _::regions_reserved = false;
  }

  bool is_region_created(uint64_t a) {
    while (_::regions_reserved) ;
    _::regions_reserved = true;

    for (const auto &pair : _::regions) {
      if (a >= pair.first && a < pair.second) {
        _::regions_reserved = false;
        return true;
      }
    }

    _::regions_reserved = false;
    return false;
  }

}

template <typename T>
inline uint8_t* to_bin(const T &__T) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  uint8_t *buff = new uint8_t[sizeof(T)];

  for (size_t i = 0; i < sizeof(T); i++) {
    buff[i] = (__T >> ((sizeof(T) - 1 - i) * 8)) & 0xFF;
  }

  return buff;
}

template <typename T>
inline std::vector<uint8_t> to_bin_v(const T &t) {
  auto bytes = to_bin(t);
  return std::vector<uint8_t>(bytes, bytes + sizeof(T));
}

WYLAND_END