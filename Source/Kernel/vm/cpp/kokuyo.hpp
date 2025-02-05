// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#pragma once

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <limits>
#include <vector>

#include <fstream>
#include <iostream>

#include "stack.hpp"

#define FLAGS_EQ (0)
#define FLAGS_LT (-1)
#define FLAGS_GT (1)

namespace kokuyo {
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

  template <typename __Ty, size_t __s>
  class array {
  public:
    __Ty data[__s];

    __Ty &operator[](size_t i) {
      if (i >= __s) throw std::out_of_range("Too large index");
      return data[i];
    }
  };

  class kokuyoVM {
  private:
    array<uint64_t, 32> regs; // 32 registers 
    size_t   ip;
    array<uint64_t, 4096> stack;
    char     flags;
    wylma::stack<uint64_t, 2048> callstack;
    bool     halt = false;

    std::vector<uint8_t> program;

    uint8_t read8() { return program[ip++]; }

    uint64_t read64() {
      uint8_t buff[8] = {read8(), read8(), read8(), read8(), read8(), read8(), read8(), read8(), };
      return cast_array<uint64_t>(buff, 8);
    }

    std::unordered_map<std::string, std::function<uint64_t(uint64_t, uint8_t *)>> stable;

    std::unordered_map<uint8_t, std::function<void()>> ftable = {
      {0x00, [this]() {}}, // NOP
      {0x01, [this]() {    // LOAD
          uint8_t r = read8();
          uint64_t data = read64();

          if (r < 32) regs[r] = data;
          else throw bad_register();
        }
      }, 
      {0x02, [this]() { // MOV
          regs[read8()] = regs[read8()];
        }
      }, 
      {0x03, [this]() { // ADD 
          regs[read8()] += regs[read8()];
        }
      },
      {0x04, [this]() { // SUB 
          regs[read8()] -= regs[read8()];
        }
      }, 
      {0x05, [this]() { // MUL
          regs[read8()] *= regs[read8()];
        }
      },
      {0x06, [this]() { // DIV 
          regs[read8()] /= regs[read8()];
        }
      },
      {0x07, [this]() { // MOD 
          uint8_t a = read8();
          uint8_t b = read8();
          regs[a] = regs[a] % regs[b];
        }
      },
      {0x08, [this]() { // JMP
          ip = read64();
        }
      },
      {0x09, [this]() { // JE 
          auto next = read64();
          ip = (flags == FLAGS_EQ ? next : ip);
        }
      }, 
      {0x0A, [this]() { // JNE
          auto next = read64();
          ip = (flags != FLAGS_EQ ? next : ip);
        }
      },
      {0x0B, [this]() { // JG 
          auto next = read64();
          ip = (flags == FLAGS_GT ? next : ip);
        }
      }, 
      {0x0C, [this]() { // JL
          auto next = read64();
          ip = (flags == FLAGS_LT ? next : ip);
        }
      },
      {0x0D, [this]() { // CMP
          auto a = read8(), b = read8();
          if (regs[a] == regs[b]) flags = FLAGS_EQ;
          else if (regs[a] > regs[b]) flags = FLAGS_GT;
          else flags = FLAGS_LT;
        }
      }, 
      {0x0E, [this]() {
          // CALL
          callstack.push(ip);
          ip = read64();
        }
      },
      {0x0F, [this]() {
          // RET
          ip = callstack.pop();
        }
      },
      {0x10, [this]() {
          // XOR
          auto a = read8(),  b = read8();
          regs[a] = regs[a] ^ regs[b];
        }
      },
      {0x11,  [this]() {
          // OR
          auto a = read8(),  b = read8();
          regs[a] = regs[a] | regs[b];
        }
      },
      {0x12,  [this]() {
          // AND
          auto a = read8(),  b = read8();
          regs[a] = regs[a] & regs[b];
        }
      },
      {0x13, [this]() {
          // SYSCALL
          std::string function_name = "";
          /* Read function’s name */
          char c;
          while (c) function_name += c;
          /* Prepare arguments */
          uint8_t argc = read8(), i = 0;
          uint8_t *argv = new uint8_t[argc];
          /* Load arguments */
          while (i < argc) {
            argv[i] = read8();
            i++;
          }
          /* Call the function */
          if (stable.find(function_name) != stable.end()) stable[function_name](argc, argv);
          else regs[0] = uint64_t(-1); /* Null pointing function, error. */
          /* Delete arguments vector */
          delete[] argv;
        }
      },
      {0x14, [this]() {
          // STORE (byte)src (qword)dst 
          auto src = read8();
          auto dst = read64();

          program[dst] = regs[src];
        }
      },
      {0x15, [this](){
          // LEA (byte)dst, (qword)src
          auto dst = read8();
          auto src = read64();
          regs[dst] = program[src];
        }
      },
      {0xFF, [this]() {
          halt = true;
        }
      }
    };

  public:
    void invoke(const std::vector<uint8_t> &in) {
      program = in;
      ip = 0x0000000000000000;
      halt = false;

      while (!halt) {
        auto c = read8();
        
        if (ftable.find(c) != ftable.end()) ftable[c]();
        else throw bad_instruction();
      }

      callstack.clear();
      program.clear();
    }

    void append_ftable(std::unordered_map<std::string, std::function<uint64_t(uint64_t, uint8_t*)>> map) {
      stable.insert(map.begin(), map.end());
    }

    void append_function(const std::string &name, std::function<uint64_t(uint64_t, uint8_t*)>  &fn) {
      stable[name] = fn;
    }
  };
} // namespace kokuyo
