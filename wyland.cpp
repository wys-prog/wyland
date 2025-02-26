#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <string>

constexpr std::size_t operator""_MB(unsigned long long size) {
  return size * 1024 * 1024;
}

constexpr std::size_t operator""_GB(unsigned long long size) {
  return size * 1024 * 1024 * 1024;
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

#define CODE_SEGMENT_SIZE 400_MB
#define HARDWARE_SEGMENT_SIZE 100_MB
#define SYSTEM_SEGMENT_SIZE 12_MB

#define CODE_SEGMENT_START 0
#define HARDWARE_SEGMENT_START (CODE_SEGMENT_START + CODE_SEGMENT_SIZE)
#define SYSTEM_SEGMENT_START (HARDWARE_SEGMENT_START + HARDWARE_SEGMENT_SIZE)

uint8_t memory[512_MB]{0};

typedef struct {
  uint8_t in; // Instruction
  uint8_t os; // Operation Size
} uir_t;

enum eflags : char {
  equal, 
  greater,
  lesser,
  // TODO
  /* overflow, 
  underflow,
  interupted,*/
};

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

enum eins {
  lea,
  load, 
  store, 
  mov, 
  add, 
  sub, 
  mul, 
  div, 
  mod, 
  jmp, 
  je, 
  jne, 
  jl, 
  jg, 
  jle, 
  jge,
  cmp,
  xint, 
};

class core {
private:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  reg_t    regs;
  bool     halted = false;
  char     flags  = 0;

  uint8_t read() {
    if (ip + 1 >= end) throw std::out_of_range("The 'end' flag is reached.");
    return memory[ip++];
  }

  template <typename T>
  T read() {
    if (ip + sizeof(T) >= end) throw std::out_of_range("The 'end' flag is reached.");
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      value |= static_cast<T>(memory[ip+i]) << ((sizeof(T) - 1 - i) * 8);
    }
    return value;
  }

  std::function<void(const uir_t&)> imov = [this](const uir_t &unpacked) {
    regs.set(regs.get(read()), read()); 
  };

  std::function<void(const uir_t&)> iadd = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) + regs.get(r2));
  };

  std::function<void(const uir_t&)> isub = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) - regs.get(r2));
  };

  std::function<void(const uir_t&)> imul = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) * regs.get(r2));
  };

  std::function<void(const uir_t&)> idiv = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) / regs.get(r2));
  };

  std::function<void(const uir_t&)> imod = [this](const uir_t &unpacked) {
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) % regs.get(r2));
  };

  std::function<void(const uir_t&)> ijmp = [this](const uir_t&) {
    ip = read<uint64_t>();
  };

  std::function<void(const uir_t&)> ije = [this](const uir_t&) {
    auto tmp = read<uint64_t>();
    if (flags == eflags::equal) ip = tmp;
  };

  std::function<void(const uir_t&)> ijne = [this](const uir_t&) {
    auto tmp = read<uint64_t>();
    if (flags != eflags::equal) ip = tmp;
  };

  std::function<void(const uir_t&)> ijg = [this](const uir_t&) {
    auto tmp = read<uint64_t>();
    if (flags == eflags::greater) ip = tmp;
  };

  std::function<void(const uir_t&)> ijl = [this](const uir_t&) {
    auto tmp = read<uint64_t>();
    if (flags == eflags::lesser) ip = tmp;
  };

  std::function<void(const uir_t&)> ijge = [this](const uir_t&) {
    auto tmp = read<uint64_t>();
    if (flags == eflags::greater || flags == eflags::equal) ip = tmp;
  };

  std::function<void(const uir_t&)> ijle = [this](const uir_t&) {
    auto tmp = read<uint64_t>();
    if (flags == eflags::lesser || flags == eflags::equal) ip = tmp;
  };

  std::function<void(const uir_t&)> icmp = [this](const uir_t &unpacked) {
    auto r1 = regs.get(read()), r2 = regs.get(read());
    if (r1 > r2) flags = eflags::greater;
    else if (r1 == r2) flags = eflags::equal;
    else flags = eflags::lesser;
  };

  std::function<void(const uir_t&)> iload = [this](const uir_t &unpacked) {
    auto size = read(), r1 = read();
    
    switch (size) {
      case 8:  regs.set(r1, read()); break;
      case 16: regs.set(r1, read<uint16_t>()); break;
      case 32: regs.set(r1, read<uint32_t>()); break;
      case 64: regs.set(r1, read<uint64_t>()); break;
      default: throw std::invalid_argument("Invalid size."); break;;
    }
  };

  std::function<void(const uir_t&)> istore = [this](const uir_t &unpacked) {
    auto size = read(), r1 = read();
    auto org = read<uint64_t>();
    auto array = to_bin(regs.get(r1));
    for (uint8_t i = 0; i < size; i++) {
      memory[org+i] = array[i];
    }
  };

  std::function<void(const uir_t&)> ixint = [this](const uir_t &unpacked) {};

  std::unordered_map<uint8_t, std::function<void(const uir_t&)>> set;

public:
  void init(uint64_t _memory_segment_begin, uint64_t _memory_segment_end) {
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    ip  = beg;
    set[eins::mov] = imov;
    set[eins::add] = iadd;
    set[eins::sub] = isub;
    set[eins::mul] = imul;
    set[eins::div] = idiv;
    set[eins::mod] = imod;
    set[eins::jmp] = ijmp;
    set[eins::je] = ije;
    set[eins::jne] = ijne;
    set[eins::jg] = ijg;
    set[eins::jl] = ijl;
    set[eins::jge] = ijge;
    set[eins::jle] = ijle;
    set[eins::cmp] = icmp;
    set[eins::load] = iload;
    set[eins::store] = istore;
    set[eins::xint] = ixint;
  }
};

std::string version() {
  return "Wyland 1.0";
}

std::string name() {
  return "Orkhon Töresi";
}

void run(std::istream &file) {
  char buffer[512]{0};
  file.read(buffer, sizeof(buffer));

  for (int i = 0; i < sizeof(buffer); i++) 
    memory[SYSTEM_SEGMENT_START+i] = buffer[i];
  
  core c;
  c.init(SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START + sizeof(buffer));
}

int main(int argc, char *const argv[]) {
  std::ifstream file;

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "--v") version();
    else if (std::string(argv[i]) == "--n") name();
    else if (std::string(argv[i]) == "-run") run(file);
    else {
      if (file.is_open()) std::cerr << "[w]: Redefining input file.";
      file.open(argv[i]);

      if (!file.is_open()) 
        std::cerr << "[e]: Unable to open file: " << argv[i] << std::endl;
    }
  }

  std::cout << version() << " —— " << name() << std::endl;

  return 0;
}