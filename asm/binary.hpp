#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <locale>
#include <codecvt>

namespace Wys {
  typedef uint8_t  UTF8; 
  typedef uint8_t  ASCII;
  typedef uint16_t UTF16;
  typedef uint32_t UTF32;

  template <typename T>
  size_t Len(const T &of) { return sizeof(T); }

  size_t Len(const UTF8 *UTF8String) {
    size_t i = 0;
    while (UTF8String[i++]);
    return i;
  }

  size_t Len(const UTF16 *UTF16String) {
    size_t i = 0;
    while (UTF16String[i++]);
    return i;
  }

  size_t Len(const UTF32 *UTF32String) {
    size_t i = 0;
    while (UTF32String[i++]);
    return i;
  }

  template <typename T>
  size_t Len(const std::basic_string<T> &str) { return str.size(); }

  template <typename T>
  std::vector<uint8_t> Serialize(T value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    std::vector<uint8_t> bytes(sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i) {
      bytes[sizeof(T) - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
    }
    return bytes;
  }

  template <typename T>
  T CastArray(const uint8_t *buff, size_t len) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    T tmp = T();
    size_t bwi = (sizeof(T) - 1) * 8;

    for (size_t i = 0; i < len && i < sizeof(T); ++i) {
      tmp |= (static_cast<T>(buff[i]) << bwi);
      bwi -= 8;
    }

    return tmp;
  }

}
