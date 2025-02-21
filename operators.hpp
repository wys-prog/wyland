#pragma once

#include <cstdint>
#include <utility>
#include "bytes.hpp"

namespace op {
  template <typename T>
  std::function<void(T&, T&)> mov = [](T &b, T &a) { a = b; };

  template <typename T>
  std::function<void(T&, T&)> add = [](T &b, T &a) { a += b; };

  template <typename T>
  std::function<void(T&, T&)> sub = [](T &b, T &a) { a -= b; };

  template <typename T>
  std::function<void(T&, T&)> mul = [](T &b, T &a) { a *= b; };

  template <typename T>
  std::function<void(T&, T&)> div = [](T &b, T &a) { a /= b; };

  template <typename T>
  std::function<void(T&, T&)> mod = [](T &b, T &a) { a = a % b; };

  class anyone {
  private:          
    uint64_t buff;

  public:           
    template <typename T>
    anyone &operator=(const T &__t) {
      buff = uint64_t(__t);
      return *this;
    }

    template <typename T>
    anyone &operator+=(const T &__t) {
      buff += uint64_t(__t);
      return *this;
    }

    template <typename T>
    anyone &operator-=(const T &__t) {
      buff -= uint64_t(__t);
      return *this;
    }

    template <typename T>
    anyone &operator*=(const T &__t) {
      buff *= uint64_t(__t);
      return *this;
    }

    template <typename T>
    anyone &operator/=(const T &__t) {
      buff /= uint64_t(__t);
      return *this;
    }

    template <typename T>
    T operator%(const T &__t) {
      return buff % uint64_t(__t);
    }
  };

} // namespace op
