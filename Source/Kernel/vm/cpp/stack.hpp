// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#pragma once

#include <stdexcept>

namespace wyland {
  template <typename T, unsigned long limit> class stack {
  private:
    T *buff;
    unsigned long capacity, size;

    void resize(unsigned long _s) {
      if (_s >= limit)
        throw std::out_of_range("too large size.");

      T *tmp = new T[_s];

      for (unsigned long i = 0; i < std::min(_s, capacity); i++) {
        tmp[i] = buff[i];
      }

      delete[] buff;
      buff = tmp;
    }

  public:
    stack() = default;

    bool try_push(const T &t) {
      if (size + 1 >= capacity) {
        try {
          resize(capacity * 0.2);
        } catch (const std::exception &e) {
          return false;
        }
      }

      buff[size++] = t;

      return true;
    }

    void push(const T &t) {
      if (!try_push(t)) throw std::underflow_error("empty stack.");
    }

    T pop() {
      if ((size - 1) != 0)
        return buff[size--];

      throw std::underflow_error("empty stack.");
      return T();
    }

    ~stack() { delete[] buff; }

    void clear() { resize(2); }

    T &operator[](unsigned long index) {
      if (index >= size) throw std::out_of_range("index out of range.");

      return buff[index];
    }
  };
} // namespace wyland
