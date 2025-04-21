#pragma once

#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <stack>
#include <new>

#include "regs.hpp"
#include "wyland-runtime/wylrt.h"
#include "interfaces/interface.hpp"
#include "libcallc.hpp"
#include "targets.h"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wformat.hpp"
#include "wtargb.hpp"

WYLAND_BEGIN

#define NAME std::string(typeid(this).name() + "::"s + __func__).c_str()

using namespace std::string_literals;
class corewtargfast : public core_base {
public:
  using syscall_t = void(corewtargfast::*)();
  using setfunc_t = void(corewtargfast::*)();

private:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  uint64_t local_ip = 0x0000000000000000;

  uint64_t regs[128]{0};
  
  bool     halted = false;
  int      flags  = 0;
  bool     is_system = false;
  uint64_t thread_id = 'U' + 'n' + 'd' + 'e' + 'f' + 'T';

  void validate_register_index(int reg) const {
    if (reg < 0 || reg >= static_cast<int>(sizeof(regs) / sizeof(regs[0]))) {
      throw std::out_of_range("Invalid register index: " + std::to_string(reg));
    }
  }

  void add(int r1, int r2) {
    try {
      validate_register_index(r1);
      validate_register_index(r2);
      regs[r1] += regs[r2];
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void sub(int r1, int r2) {
    try {
      validate_register_index(r1);
      validate_register_index(r2);
      regs[r1] -= regs[r2];
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void mul(int r1, int r2) {
    try {
      validate_register_index(r1);
      validate_register_index(r2);
      regs[r1] *= regs[r2];
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void div(int r1, int r2) {
    try {
      validate_register_index(r1);
      validate_register_index(r2);
      if (regs[r2] == 0) {
        throw std::runtime_error("Division by zero");
      }
      regs[r1] /= regs[r2];
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void mod(int r1, int r2) {
    try {
      validate_register_index(r1);
      validate_register_index(r2);
      if (regs[r2] == 0) {
        throw std::runtime_error("Modulo by zero");
      }
      regs[r1] %= regs[r2];
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void mov(int r1, uint64_t value) {
    try {
      validate_register_index(r1);
      regs[r1] = value;
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void cmp(int r1, int r2) {
    try {
      validate_register_index(r1);
      validate_register_index(r2);
      if (regs[r1] == regs[r2]) {
        flags = 0; // Equal
      } else if (regs[r1] > regs[r2]) {
        flags = 1; // Greater
      } else {
        flags = -1; // Lesser
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void jmp(uint64_t address) {
    try {
      if (address < beg || address >= end) {
        throw std::out_of_range("Jump address out of bounds: " + std::to_string(address));
      }
      ip = address;
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void je(uint64_t address) {
    try {
      if (flags == 0) {
        jmp(address);
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void jne(uint64_t address) {
    try {
      if (flags != 0) {
        jmp(address);
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void jg(uint64_t address) {
    try {
      if (flags == 1) {
        jmp(address);
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void jl(uint64_t address) {
    try {
      if (flags == -1) {
        jmp(address);
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void jge(uint64_t address) {
    try {
      if (flags == 1 || flags == 0) {
        jmp(address);
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

  void jle(uint64_t address) {
    try {
      if (flags == -1 || flags == 0) {
        jmp(address);
      }
    } catch (const std::exception &e) {
      std::cerr << "[e]: Error in " << NAME << "(): " << e.what() << std::endl;
      throw;
    }
  }

public:
  void init(uint64_t, 
            uint64_t, 
            bool , 
            uint64_t, 
            linkedfn_array *, 
            uint64_t, IWylandGraphicsModule * = nullptr,
            WylandMMIOModule* = nullptr, WylandMMIOModule* = nullptr, 
            WylandMMIOModule * = nullptr) override {
  
  }

  void run() override {
    
  }
};

WYLAND_END