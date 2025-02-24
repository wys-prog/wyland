#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>

constexpr std::size_t operator""_MB(unsigned long long size) {
  return size * 1024 * 1024;
}

constexpr std::size_t operator""_GB(unsigned long long size) {
  return size * 1024 * 1024 * 1024;
}

uint8_t memory[512_MB]{0};

template <typename _Res, typename... _ArgTypes>
using lambda = std::function<_Res(_ArgTypes...)>;

lambda<uint64_t, uint64_t, uint64_t> op_mov = [](uint64_t, uint64_t b)   { return b;     };
lambda<uint64_t, uint64_t, uint64_t> op_add = [](uint64_t a, uint64_t b) { return a + b; };
lambda<uint64_t, uint64_t, uint64_t> op_sub = [](uint64_t a, uint64_t b) { return a - b; };
lambda<uint64_t, uint64_t, uint64_t> op_mul = [](uint64_t a, uint64_t b) { return a * b; };
lambda<uint64_t, uint64_t, uint64_t> op_div = [](uint64_t a, uint64_t b) { return a / b; };
lambda<uint64_t, uint64_t, uint64_t> op_mod = [](uint64_t a, uint64_t b) { return a % b; };

typedef struct {
  uint8_t in; // Instruction
  uint8_t os; // Instructions Size
  uint8_t ot; // Operators Type
} uir_t;


class core {
private:
  uint8_t  r8 [16]{0};
  uint16_t r16[16]{0};
  uint32_t r32[16]{0};
  uint64_t r64[16]{0};
  bool     halted = false;

  std::function<void()> ds;

};

int main() {
  return 0;
}