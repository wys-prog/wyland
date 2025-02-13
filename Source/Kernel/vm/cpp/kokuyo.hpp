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

namespace wyland {
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

  class invalid_register : public std::exception {};
  class bad_instruction  : public std::exception {};

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
    std::vector<std::string> tracer{"Tracer created"};
    uint64_t i = 0;

    array<uint64_t, 32> regs; // 32 registers 
    uint64_t ip;
    array<uint64_t, 4096> stack;
    char flags;
    wylma::stack<uint64_t, 2048> callstack;
    bool halt = false;

    std::vector<uint8_t> program;

    void testreg(uint8_t r) {
      if (r >= 32) {
        tracer.push_back("Invalid register. @r is too big (max: 32)");
        tracer.push_back("Given @r: " + std::to_string((int)r));
        throw invalid_register();
      }
    }

    void testaddr(uint64_t a) {
      if (a > program.size()) {
        tracer.push_back("Index is out of virtual memory size.");
        tracer.push_back("Address: " + std::to_string(a) + " size: " + std::to_string(program.size()));
        throw std::out_of_range("Too big address");
      }
    }

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

          testreg(r);
          regs[r] = data;
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
          auto next = read64();  
          testaddr(next);
          ip = next;
        }
      },
      {0x09, [this]() { // JE 
          auto next = read64();
          testaddr(next);
          ip = (flags == FLAGS_EQ ? next : ip);
        }
      }, 
      {0x0A, [this]() { // JNE
          auto next = read64();
          testaddr(next);
          ip = (flags != FLAGS_EQ ? next : ip);
        }
      },
      {0x0B, [this]() { // JG 
          auto next = read64();
          testaddr(next);
          ip = (flags == FLAGS_GT ? next : ip);
        }
      }, 
      {0x0C, [this]() { // JL
          auto next = read64();
          testaddr(next);
          ip = (flags == FLAGS_LT ? next : ip);
        }
      },
      {0x0D, [this]() { // CMP
          auto a = read8(), b = read8();
          testreg(a); testreg(b);
          if (regs[a] == regs[b]) flags = FLAGS_EQ;
          else if (regs[a] > regs[b]) flags = FLAGS_GT;
          else flags = FLAGS_LT;
        }
      }, 
      {0x0E, [this]() {
          // CALL
          callstack.push(ip);
          auto next = read64();
          testaddr(next);
          ip = next;
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
          testreg(a); testreg(b);
          regs[a] = regs[a] ^ regs[b];
        }
      },
      {0x11,  [this]() {
          // OR
          auto a = read8(),  b = read8();
          testreg(a); testreg(b);
          regs[a] = regs[a] | regs[b];
        }
      },
      {0x12,  [this]() {
          // AND
          auto a = read8(),  b = read8();
          testreg(a); testreg(b);
          regs[a] = regs[a] & regs[b];
        }
      },
      {0x13, [this]() {
          // SYSCALL
          std::string function_name = "";
          /* Read function’s name */
          char c = read8();
          do {
            function_name.push_back(c);
            c = (char)read8();
          } while (c);
          /* Prepare arguments */
          uint64_t argc = read64(), i = 0;
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

          testreg(src);
          testaddr(dst);

          program[dst] = regs[src];
        }
      },
      {0x15, [this](){
          // LEA (byte)dst, (qword)src
          auto dst = read8();
          auto src = read64();

          testreg(dst);
          testaddr(src);
          regs[dst] = program[src];
        }
      },
      {0xFF, [this]() {
          halt = true;
        }
      }
    };

  public:
    void invoke(const std::vector<uint8_t> &in, uint64_t _ip = 0x0000000000000000) {
      tracer.push_back("Invoked");
      program = in;
      ip = _ip;
      halt = false;

      while (!halt) {
        auto c = read8();
        
        if (ftable.find(c) != ftable.end()) ftable[c]();
        else {
          tracer.push_back("Bad instruction: " + std::to_string(int(c)));
          tracer.push_back("IP: " + std::to_string(ip));
          throw bad_instruction();
        }
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

    std::vector<std::string> &get_trace() {
      return tracer;
    }

    uint64_t get_ip() {
      return ip;
    }
  };
} // namespace wyland
