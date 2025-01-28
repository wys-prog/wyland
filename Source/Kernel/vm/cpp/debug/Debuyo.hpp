// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#include <cstdint>
#include <functional>
#include <sstream>
#include <stdfloat>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <fstream>
#include <iostream>

#include "stack.hpp"

#define FLAGS_EQ (0)
#define FLAGS_LT (-1)
#define FLAGS_GT (1)

template <typename T>
T cast_array(uint8_t *buff, uint64_t len) {
  T tmp = T(); 
  int bwi = (sizeof(T) - 1) * 8; 
  
  for (uint64_t i = 0; i < len && i < sizeof(T); ++i) {
    tmp |= (static_cast<T>(buff[i]) << bwi); 
    bwi -= 8;
  }

  return tmp;
}

class bad_register    : public std::exception {};
class bad_instruction : public std::exception {};
class stack_overflow  : public std::exception {};


class Dobuyo {
private:
  std::ostream &os;
  uint64_t regs[32]; // 32 registers 
  size_t   ip;
  uint64_t stack[4096];
  char     flags;
  wylma::stack<uint64_t, 2048> callstack;
  bool     halt = false;

  std::vector<uint8_t> program;

  uint8_t read8() { return program[ip++]; }

  uint64_t read64() {
    uint8_t buff[8] = {read8(), read8(), read8(), read8(), read8(), read8(), read8(), read8(), };
    return cast_array<uint64_t>(buff, 8);
  }

  std::unordered_map<uint8_t, std::function<void()>> ftable = {
    {0x00, [this]() {}}, // NOP
    {0x01, [this]() {    // LOAD
        uint8_t r = read8();
        uint64_t data = read64();

        if (r < 32) {
          os << "LOAD: R" << r << ", " << data << std::endl;
          regs[r] = data;
        } else {
          os << "LOAD fails. Bad register: " << r << std::endl;
          throw bad_register();
        }
      }
    }, 
    {0x02, [this]() { // MOV
        auto a = read8(), b = read8();
        regs[a] = regs[b];
        os << "MOV R" << a << ", R" << b
        << "\nResult: " << regs[a] << std::endl;
      }
    }, 
    {0x03, [this]() { // ADD 
        auto a = read8(), b = read8();
        regs[a] += regs[b];
        os << "ADD R" << a << ", R" << b
        << "\nResult: " << regs[a] << std::endl;
      }
    },
    {0x04, [this]() { // SUB 
        auto a = read8(), b = read8();
        regs[a] -= regs[b];
        os << "SUB R" << a << ", R" << b
        << "\nResult: " << regs[a] << std::endl;
      }
    }, 
    {0x05, [this]() { // MUL
        auto a = read8(), b = read8();
        regs[a] *= regs[b];
        os << "MUL R" << a << ", R" << b
        << "\nResult: " << regs[a] << std::endl;
      }
    },
    {0x06, [this]() { // DIV 
        auto a = read8(), b = read8();
        regs[a] /= regs[b];
        os << "DIV R" << a << ", R" << b 
        << "\nResult: " << regs[a] << std::endl;
      }
    },
    {0x07, [this]() { // MOD 
        uint8_t a = read8();
        uint8_t b = read8();
        regs[a] = regs[a] % regs[b];
        os << "MOD R" << a << ", R" << b
           << "\nResult: " << regs[a] << std::endl;
      }
    },
    {0x08, [this]() { // JMP
        auto next = read64();
        os << "JUMP " << next;
        ip = next;
      }
    },
    {0x09, [this]() { // JE 
        auto next = read64();
        ip = (flags == FLAGS_EQ ? next : ip);
        os << "JE " << next << (flags == FLAGS_EQ ? " (jumped)" : " (not jumped)") << std::endl;
      }
    }, 
    {0x0A, [this]() { // JNE
        auto next = read64();
        ip = (flags != FLAGS_EQ ? next : ip);
        os << "JNE " << next << (flags != FLAGS_EQ ? " (jumped)" : " (not jumped)") << std::endl;
      }
    },
    {0x0B, [this]() { // JG 
        auto next = read64();
        ip = (flags == FLAGS_GT ? next : ip);
        os << "JG " << next << (flags == FLAGS_GT ? " (jumped)" : " (not jumped)") << std::endl;
      }
    }, 
    {0x0C, [this]() { // JL
        auto next = read64();
        ip = (flags == FLAGS_LT ? next : ip);
        os << "JL " << next << (flags == FLAGS_LT ? " (jumped)" : " (not jumped)") << std::endl;
      }
    },
    {0x0D, [this]() { // CMP
        auto a = read8(), b = read8();
        if (regs[a] == regs[b]) flags = FLAGS_EQ;
        else if (regs[a] > regs[b]) flags = FLAGS_GT;
        else flags = FLAGS_LT;
        os << "CMP : Flags = " << (int)flags << std::endl;
      }
    }, 
    {0x0E, [this]() {
        // CALL
        callstack.push(ip);
        auto next = read64();
        os << "CALL " << next << "\n";
        ip = next;
      }
    },
    {0x0F, [this]() {
        // RET
        ip = callstack.pop();
        os << "RET : Returning to " << ip << std::endl;
      }
    },
    {0x10, [this]() {
        // XOR
        auto a = read8(),  b = read8();
        regs[a] = regs[a] ^ regs[b];
        os << "XOR R" << a << ", R" << b 
        << "\nResult: " << regs[a]
        << std::endl;
      }
    },
    {0x11,  [this]() {
        // OR
        auto a = read8(),  b = read8();
        regs[a] = regs[a] | regs[b];
        os << "OR R" << a << ", R" << b 
        << "\nResult: " << regs[a]
        << std::endl;
      }
    },
    {0x12,  [this]() {
        // AND
        auto a = read8(),  b = read8();
        regs[a] = regs[a] & regs[b];
        os << "AND R" << a << ", R" << b 
        << "\nResult: " << regs[a]
        << std::endl;
      }
    },
    {0xFF, [this]() {
        halt = true;
        os << "HALT" << std::endl;
      }
    }
  };

public:
  void invoke(const std::vector<uint8_t> &in) {
    program = in;
    ip = 0x0000000000000000;
    halt = false;
  }

  void step() {
    auto c = read8();

    if (ftable.find(c) != ftable.end()) ftable[c]();
    else throw bad_instruction();
  }

  void end() {
    callstack.clear();
    program.clear();
  }

  bool is_halted() { return halt; }

  std::string get_stack() {
    std::stringstream ss;
    
    for (int i = 0; i < 4096; ++i) 
      ss << std::uppercase << std::hex << stack[i] << '\n';
    
    return ss.str();
  }

  std::string get_registers() {
    std::stringstream ss;

    for (int i = 0; i < 32; i++) 
      ss << std::uppercase << std::hex << regs[i] << '\n';
    
    return ss.str();
  }

  std::string get_callstack() {
    std::stringstream ss;
    
    for (int i = 0; i < 1024; ++i) 
      ss << callstack[i] << '\n';
    
    return ss.str();
  }

  uint64_t get_memory() {
    return (program.size());
  }

  uint64_t get_ip() {
    return (ip);
  }
};