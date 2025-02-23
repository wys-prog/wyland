#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include "bytes.hpp"

namespace op {

  class anyone {
  private:
    uint64_t buff;

  public:
    anyone() : buff(0) {}

    template <typename T>
    anyone(const T &__t) : buff((uint64_t)__t) {}

    template <typename T>
    anyone &operator=(const T &__t) {
      buff = static_cast<uint64_t>(__t);
      return *this;
    }

    template <typename T>
    anyone &operator+=(const T &__t) {
      buff += static_cast<uint64_t>(__t);
      return *this;
    }

    template <typename T>
    anyone &operator-=(const T &__t) {
      buff -= static_cast<uint64_t>(__t);
      return *this;
    }

    template <typename T>
    anyone &operator*=(const T &__t) {
      buff *= static_cast<uint64_t>(__t);
      return *this;
    }

    template <typename T>
    anyone &operator/=(const T &__t) {
      buff /= static_cast<uint64_t>(__t);
      return *this;
    }

    template <typename T>
    T operator%(const T &__t) {
      return buff % static_cast<uint64_t>(__t);
    }

    uint64_t get() const {
      return buff;
    }
  };

  template <typename TA, typename TB>
  void anyme(TA &__tA, TB &__tB, const std::function<void(anyone&, anyone&)> &fn) {
    anyone me(__tA), another(__tB);
    fn(me, another);
    __tA = (TA)me.get();
    __tB = (TB)another.get();
  }

  std::function<void(anyone &a, anyone &b)> mov = [](anyone &a, anyone &b) { a = b.get(); };
  std::function<void(anyone &a, anyone &b)> add = [](anyone &a, anyone &b) { a += b.get(); };
  std::function<void(anyone &a, anyone &b)> sub = [](anyone &a, anyone &b) { a -= b.get(); };
  std::function<void(anyone &a, anyone &b)> mul = [](anyone &a, anyone &b) { a *= b.get(); };
  std::function<void(anyone &a, anyone &b)> div = [](anyone &a, anyone &b) { a /= b.get(); };
  std::function<void(anyone &a, anyone &b)> mod = [](anyone &a, anyone &b) { a = a.get() % b.get(); };
  std::function<void(anyone &a, anyone &b)> xor_op = [](anyone &a, anyone &b) { a = a.get() ^ b.get(); };
  std::function<void(anyone &a, anyone &b)> and_op = [](anyone &a, anyone &b) { a = a.get() & b.get(); };
  std::function<void(anyone &a, anyone &b)> or_op = [](anyone &a, anyone &b) { a = a.get() | b.get(); };

  template <typename T>
  T compare(const T &a, const T &b) {
    if (a > b) return T(1);
    else if (b > a) return T(-1);
    else if (a == b) return T(0);
    return T(2);
  }
} // namespace op

  typedef std::function<void(op::anyone&, op::anyone&)> _fnOP;
  typedef op::anyone any;
