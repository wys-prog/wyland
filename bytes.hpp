#pragma once

#include <cstdint>
#include <type_traits>
#include <cstring> 

namespace bytemanip {

  template <typename T>
  uint8_t* to_bin(const T &__T) {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    uint8_t *buff = new uint8_t[sizeof(T)];

    for (size_t i = 0; i < sizeof(T); i++) {
      buff[i] = (__T >> ((sizeof(T) - 1 - i) * 8)) & 0xFF;
    }

    return buff;
  }

  template <typename T>
  T from_bin(const uint8_t *buff) {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      value |= static_cast<T>(buff[i]) << ((sizeof(T) - 1 - i) * 8);
    }

    return value;
  }

} // namespace bytemanip
