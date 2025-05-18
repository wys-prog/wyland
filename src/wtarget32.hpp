#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

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
#include <new>

#include "regs.hpp"
#include "wyland-runtime/wylrt.h"
#include "libcallc.hpp"
#include "targets.h"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wformat.hpp"
#include "wtargb.hpp"

#include "bios/bios.hpp"
#include "interfaces/interface.hpp"

WYLAND_BEGIN

class corewtarg32 : public wyland_base_core {
  using syscall_t = void(corewtarg32::*)();
  using setfunc_t = void(corewtarg32::*)();

private:
  uint32_t beg = 0x00000000;
  uint32_t end = 0xFFFFFFFF;
  uint32_t ip  = 0x00000000;
  uint32_t local_ip = 0x00000000;
  reg_t    regs;
  bool     halted = false;
  int      flags  = 0;
  bool     is_system = false;
  uint64_t thread_id = 'U' + 'n' + 'd' + 'e' + 'f' + 'T';
  uint8_t  children = 0;
  std::mutex mtx;
  std::condition_variable cv;
  std::unordered_map<uint32_t, libcallc::DynamicLibrary> libs;
  std::unordered_map<uint32_t, libcallc::DynamicLibrary::FunctionType> funcs;

  uint8_t read() {
    if (ip + 1 >= end) throw std::out_of_range("The 'end' flag is reached.\n"
      "Thread: " + std::to_string(thread_id));
    return memory[ip++];
  }

  template <typename T>
  T read() {
    if (ip + sizeof(T) > end) throw std::out_of_range("The 'end' flag is reached\n"
      "Thread: " + std::to_string(thread_id));

    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      value |= static_cast<T>(memory[ip + i]) << ((sizeof(T) - 1 - i) * 8);
    }

    ip += sizeof(T);
    return value;
  }

  void imov() {
    regs.set(regs.get(read()), read()); 
  };

  void iadd() {
    auto r1 = read(), r2 = read(); 
    auto v1 = regs.get(r1) + regs.get(r2);
    regs.set(r1, v1);
  };

  void isub() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) - regs.get(r2));
  };

  void imul() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) * regs.get(r2));
  };

  void idiv() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) / regs.get(r2));
  };

  void imod() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r1) % regs.get(r2));
  };

  void ijmp() {
    ip = read<uint32_t>();
  };

  void ije() {
    auto tmp = read<uint32_t>();
    if (flags == 0) ip = tmp;
  };

  void ijne() {
    auto tmp = read<uint32_t>();
    if (flags == 0) return;
    ip = tmp;
  };

  void ijg() {
    auto tmp = read<uint32_t>();
    if (flags == 1) ip = tmp;
  };

  void ijl() {
    auto tmp = read<uint32_t>();
    if (flags == -1) ip = tmp;
  };

  void ijge() {
    auto tmp = read<uint32_t>();
    if (flags == 1 || flags == 0) ip = tmp;
  };

  void ijle() {
    auto tmp = read<uint32_t>();
    if (flags == -1 || flags == 0) ip = tmp;
  };

  void icmp() {
    auto r1 = regs.get(read());
    auto r2 = regs.get(read());
  
    if (r1 > r2) {
      flags = 1;
    } else if (r1 == r2) {
      flags = 0;
    } else {
      flags = -1;
    }
  };

  void iload() {
    auto size = read();
    auto r1 = read();

    switch (size) {
      case 8:  regs.set(r1, read<uint8_t>()); break;
      case 16: regs.set(r1, read<uint16_t>()); break;
      case 32: regs.set(r1, read<uint32_t>()); break;
      case 64: regs.set(r1, read<uint64_t>()); break;
      default: throw std::invalid_argument("in iload(): Invalid size: " + std::to_string(size)
        + "\n\tfetched: [" + std::to_string((int)size) + "]"
          "\n\tThread:  " + std::to_string(thread_id) 
        + "\n\tIP:      " + std::to_string(ip)
        + "\n\tLIP:     " + std::to_string(local_ip)
        + "\n\tARGS:    [" + std::to_string((int)size) + ", " + std::to_string((int)r1) +  "]"); 
      break;
    }
    
  };

  void istore() {
    auto size = read() / 8;
    auto r1 = read();
    auto org = read<uint32_t>();
    auto array = to_bin(regs.get(r1));
    for (uint8_t i = 0; i < size; i++) {
      memory[org+i] = array[i];
    }
    delete[] array;
  };

  void ixint() {
    
  };

  void nop() {};

  void ilea() {
    auto r1 = read();
    auto ad = read<uint32_t>();

    if (ad >= (SYSTEM_SEGMENT_START + SYSTEM_SEGMENT_SIZE) && !is_system) 
      throw std::runtime_error("Permission denied: Accessing system's segment.\n"
      "Thread: " + std::to_string(thread_id));
    
    if (ad >= HARDWARE_SEGMENT_START) {
      while (global::keyboard_reserved);
      global::keyboard_reserved = true;
    }
    
    regs.set(r1, ad);

    if (ad >= HARDWARE_SEGMENT_START) global::keyboard_reserved = false;
  }

  void iloadat() {
    auto dst = read();
    auto at = read<uint32_t>(); /* This function will at memory[at]. */
    
    if (at >= (SYSTEM_SEGMENT_START + SYSTEM_SEGMENT_SIZE) && !is_system) 
    throw std::runtime_error("Permission denied: Accessing system's segment.\n"
    "Thread: " + std::to_string(thread_id));
  
    if (at >= HARDWARE_SEGMENT_START) {
      while (global::keyboard_reserved);
      global::keyboard_reserved = true;
    }
    
    regs.set(dst, memory[at]);

    if (at >= HARDWARE_SEGMENT_START) global::keyboard_reserved = false;
  }

  void iret() { ip = regs.get(63); }

  void imovad() {
    auto a = read(), b = read();
    regs.set(a, memory[regs.get(b)]);
  }

  setfunc_t set[22];

public:
  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system, 
            uint64_t _name, 
            linkedfn_array *, 
            uint64_t, IWylandGraphicsModule* = nullptr, 
            WylandMMIOModule* = nullptr, WylandMMIOModule* = nullptr, 
            WylandMMIOModule* = nullptr, BIOS* = nullptr) override {
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    ip  = beg;
    is_system = _is_system;
    thread_id = _name;

    set[set_arch64::nop] = &corewtarg32::nop; /* Instruction set is the same. Only OP's size changes.*/
    set[set_arch64::lea] = &corewtarg32::ilea; 
    set[set_arch64::load] = &corewtarg32::iload;
    set[set_arch64::store] = &corewtarg32::istore;
    set[set_arch64::mov] = &corewtarg32::imov;
    set[set_arch64::add] = &corewtarg32::iadd;
    set[set_arch64::sub] = &corewtarg32::isub;
    set[set_arch64::mul] = &corewtarg32::imul;
    set[set_arch64::odiv] = &corewtarg32::idiv;
    set[set_arch64::mod] = &corewtarg32::imov;
    set[set_arch64::jmp] = &corewtarg32::ijmp;
    set[set_arch64::je] = &corewtarg32::ije;
    set[set_arch64::jne] = &corewtarg32::ijne;
    set[set_arch64::jl] = &corewtarg32::ijl;
    set[set_arch64::jg] = &corewtarg32::ijg;
    set[set_arch64::jle] = &corewtarg32::ijle;
    set[set_arch64::jge] = &corewtarg32::ijge;
    set[set_arch64::cmp] = &corewtarg32::icmp;
    set[set_arch64::xint] = &corewtarg32::ixint;
    set[set_arch64::loadat] = &corewtarg32::iloadat;
    set[set_arch64::ret]    = &corewtarg32::iret;
    set[set_arch64::movad]  = &corewtarg32::imovad;
  }

  void run() override {
    while (!halted) {
      
      if (ip < beg || ip > end) 
        throw std::out_of_range(
          "Reading out of the local segment.\n"
          "\tflag 'beg':\t" + std::to_string(beg) + "\n"
          "\tflag 'end':\t" + std::to_string(end) + "\n"
          "\tIP (global):\t" + std::to_string(ip)
        );
      auto fetched =  read();
      local_ip++;
      
      if (fetched == 0xFF) { halted = true; continue; }

      if (fetched >= sizeof(set) / sizeof(set[0])) {
        std::ostringstream oss;
        oss << "Invalid instruction: " << std::hex << std::uppercase << (int)fetched << "\n"
        "\tfetched:\t[" << (int)fetched<< "]\n"
        "\t4 bytes:\t[" <<  
        format({memory[ip-2],memory[ip-1],memory[ip],memory[ip+1]}, ',') 
        << "]\n"
        "\tCore IP:\t" << ip << std::dec << "\n"
        "\tLocal IP:\t" << local_ip << "\n\tThread: " << thread_id;
        throw std::runtime_error(oss.str());
      }

      (this->*set[fetched])();

      if (beg + 1 >= end) throw std::out_of_range("Reading out of segment.\n"
        "\tThread: " + std::to_string(thread_id));
    }

    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this] { return children == 0; });
    }
  }
};

WYLAND_END