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

typedef struct {
  uint8_t in; // Instruction
  uint8_t os; // Operation Size
} uir_t;

class reg_t {
private:
  uint8_t  r8 [16]{0};
  uint16_t r16[16]{0};
  uint32_t r32[16]{0};
  uint64_t r64[16]{0};

public:
  void set(uint16_t to, uint64_t u) {
    if (to < 16) r8[to] = u;
    else if (to < 32) r16[to] = u;
    else if (to < 48) r32[to] = u;
    else if (to < 64) r64[to] = u;
    else throw std::runtime_error("Unexpected register: " + std::to_string(to));
  }

  uint64_t get(uint16_t who) {
    if (who < 16) return r8[who];
    else if (who < 32) return r16[who];
    else if (who < 48) return r32[who];
    else if (who < 64) return r64[who];
    else throw std::runtime_error("Unexpected register: " + std::to_string(who));
    return -1;
  }
};

class core {
private:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  reg_t    regs;
  bool     halted = false;

  uint8_t read() {
    if (ip + 1 >= end) throw std::out_of_range("The 'end' flag is reached.");
    return memory[ip++];
  }

  std::function<void(const uir_t&)> imov = [this](const uir_t &unpacked) {
    regs.set(regs.get(read()), read()); 
  };

  std::function<void(const uir_t&)> iadd = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(1) + regs.get(2));
  };

  std::function<void(const uir_t&)> isub = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(1) - regs.get(2));
  };

  std::function<void(const uir_t&)> imul = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(1) * regs.get(2));
  };

  std::function<void(const uir_t&)> idiv = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(1) / regs.get(2));
  };

  std::function<void(const uir_t&)> imod = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(1) % regs.get(2));
  };

  
};

int main() {
  return 0;
}