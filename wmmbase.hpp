#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

#define CODE_SEGMENT_SIZE 400_MB
#define HARDWARE_SEGMENT_SIZE 100_MB
#define SYSTEM_SEGMENT_SIZE 12_MB

#define CODE_SEGMENT_START 0
#define HARDWARE_SEGMENT_START (CODE_SEGMENT_START + CODE_SEGMENT_SIZE)
#define SYSTEM_SEGMENT_START (HARDWARE_SEGMENT_START + HARDWARE_SEGMENT_SIZE)

#define KEYBOARD_SEGMENT_START HARDWARE_SEGMENT_START
#define KEYBOARD_SEGMENT_END   HARDWARE_SEGMENT_START + 2_MB 

#define SYSCALL_COUNT 11

constexpr std::size_t operator""_MB(unsigned long long size) {
  return size * 1024 * 1024;
}

constexpr std::size_t operator""_GB(unsigned long long size) {
  return size * 1024 * 1024 * 1024;
}


uint8_t memory[512_MB]{0};

namespace segments {
  static bool keyboard_reserved;
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