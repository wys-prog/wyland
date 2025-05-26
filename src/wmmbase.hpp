#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <bit>

#define WYLAND_BEGIN namespace wylma { namespace wyland { 
#define WYLAND_END   } }
#define ARCH_BACK(n) namespace n  {
#define ARCH_BACK_V(v) namespace v {
#define ARCH_END }

#define SYSTEM_SEGMENT_SIZE 12_MB

#define SYSTEM_SEGMENT_START 0
#define HARDWARE_SEGMENT_START (SYSTEM_SEGMENT_START + SYSTEM_SEGMENT_SIZE)

#define KEYBOARD_SEGMENT_START HARDWARE_SEGMENT_START
#define KEYBOARD_SEGMENT_END   HARDWARE_SEGMENT_START + 2_MB 

#define WYLAND_SYSTEM_INT_BEG 10_MB

#define WYLAND_MEMORY_MINIMUM 16_MB
#define WYLAND_STD_USB_PORTSC 8 

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

#define BLOCK_SIZE_BYTES (4096)
#define WUINTS_PER_BLOCK (BLOCK_SIZE_BYTES / sizeof(wuint))

#ifdef _WIN32
#define STRDUP(x) _strdup(x)
#else 
#define STRDUP(x) strdup(x)
#endif // _WIN32

#define mnameof(x) #x
#define memberofstr std::string(typeid(this).name() + "::"s + __func__)
#define memberofcstr (std::string(typeid(this).name() + "::"s + __func__).c_str())

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

using namespace std::string_literals;


[[maybe_unused]] extern uint8_t *memory;
[[maybe_unused]] extern uint64_t code_start;

namespace global {
  [[maybe_unused]] static bool keyboard_reserved;
  [[maybe_unused]] static uint64_t memory_size = WYLAND_MEMORY_MINIMUM;
}

class GlobalSettings {
public:
  static bool print_specs;
};

#ifdef ___WYLAND_GNU_USE_FLOAT128___
typedef __float128 __wyland_long_float;
#include <quadmath.h>

inline std::ostream& operator<<(std::ostream& os, __float128 value) {
  char buffer[128];
  quadmath_snprintf(buffer, sizeof(buffer), "%.36Qg", value);
  os << buffer;
  return os;
}
#else 
typedef long double __wyland_long_float;
#endif // ? ___WYLAND_GNU_USE_FLOAT128___



namespace wyland_terminate_data {
  typedef void(*wyscppecllb)(void);
  extern std::vector<wyscppecllb> end_callables;
}

#ifdef ___NO___

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

#endif // ___NO___

template <typename T>
inline uint8_t* to_bin(const T &obj) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  uint8_t *buff = new uint8_t[sizeof(T)];

  for (size_t i = 0; i < sizeof(T); i++) {
    buff[i] = (obj >> ((sizeof(T) - 1 - i) * 8)) & 0xFF;
  }

  return buff;
}

template <typename T>
inline std::vector<uint8_t> to_bin_v(const T &t) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

    // Direct memory reinterpretation (no loop, no dynamic alloc)
  const auto *ptr = reinterpret_cast<const uint8_t*>(&t);

    // Big-endian correction if necessary (assuming target is little-endian)
  if constexpr (std::endian::native == std::endian::little) {
    return {ptr, ptr + sizeof(T)};
  } else {
    // Reverse for big-endian targets
    return std::vector<uint8_t>(std::make_reverse_iterator(ptr + sizeof(T)),
                                std::make_reverse_iterator(ptr));
  }
}

WYLAND_END