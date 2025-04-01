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
#include "libcallc.hpp"
#include "targets.h"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wformat.hpp"
#include "wtargb.hpp"

WYLAND_BEGIN

class corewtargfast : public core_base {
public:
  typedef struct {
    uint16_t op;
    uint8_t  param1;
    uint8_t  param2;
    uint8_t  param3;
    uint64_t imm;
  } wtfins; /* Wyland Target Fast - Instruction*/

  using syscall_t = void(corewtargfast::*)();
  using setfunc_t = void(corewtargfast::*)();

private:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  uint64_t local_ip = 0x0000000000000000;

  uint64_t regs[128]{0};
  
  std::stack<wtfins> pipeline;
  std::mutex         mpipe;

  bool     halted = false;
  int      flags  = 0;
  bool     is_system = false;
  uint64_t thread_id = 'U' + 'n' + 'd' + 'e' + 'f' + 'T';
  uint8_t  children = 0;
  std::mutex mtx;
  std::condition_variable cv;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary> libs;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary::FunctionType> funcs;

  void jump_if(const wtfins &wtf) {
    switch (wtf.param3) {
      case 0x00: 
        if (regs[wtf.param1] == regs[wtf.param2]) 
          ip = wtf.imm;
        break;
      case 0x01: 
        if (regs[wtf.param1] > regs[wtf.param2]) 
          ip = wtf.imm;
        break;
      case 0x02: 
        if (regs[wtf.param1] < regs[wtf.param2]) 
          ip = wtf.imm;
        break;
      
      default: 
        throw std::invalid_argument("jump_if(): No such parameter for %param3: " + std::to_string((int)wtf.param3));
        break;
    }
  }

  void jump(const wtfins &wtf) {
    ip = wtf.imm;
  }

  void add(const wtfins &wtf) {
    regs[wtf.param3] = regs[wtf.param1] + regs[wtf.param2];
  }

  void sub(const wtfins &wtf) {
    regs[wtf.param3] = regs[wtf.param1] - regs[wtf.param2];
  }

  void mul(const wtfins &wtf) {
    regs[wtf.param3] = regs[wtf.param1] * regs[wtf.param2];
  }

  void div(const wtfins &wtf) {
    regs[wtf.param3] = regs[wtf.param1] / regs[wtf.param2];
  }

  void mod(const wtfins &wtf) {
    regs[wtf.param3] = regs[wtf.param1] % regs[wtf.param2];
  }



public:
  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system, 
            uint64_t _name, 
            linkedfn_array *table) override {
  
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    is_system = _is_system;
    thread_id = _name;
  }

  void run() override {
    
  }
};

WYLAND_END