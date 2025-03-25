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
#include <new>

#include "wyland-runtime/keys.h"
#include "wyland-runtime/wylrt.h"
#include "wyland-runtime/wylrt.hpp"

#include "regs.hpp"
#include "libcallc.hpp"
#include "targets.h"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wformat.hpp"
#include "wtargb.hpp"

WYLAND_BEGIN

class corewtarg64 : public core_base {
  using syscall_t = void(corewtarg64::*)();
  using setfunc_t = void(corewtarg64::*)();

private:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  uint64_t local_ip = 0x0000000000000000;
  reg_t    regs;
  bool     halted = false;
  int      flags  = 0;
  bool     is_system = false;
  uint64_t thread_id = 'U' + 'n' + 'd' + 'e' + 'f' + 'T';
  uint8_t  children = 0;
  std::mutex mtx;
  std::condition_variable cv;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary> libs;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary::FunctionType> funcs;

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
    ip = read<uint64_t>();
  };

  void ije() {
    auto tmp = read<uint64_t>();
    if (flags == 0) ip = tmp;
  };

  void ijne() {
    auto tmp = read<uint64_t>();
    if (flags == 0) return;
    ip = tmp;
  };

  void ijg() {
    auto tmp = read<uint64_t>();
    if (flags == 1) ip = tmp;
  };

  void ijl() {
    auto tmp = read<uint64_t>();
    if (flags == -1) ip = tmp;
  };

  void ijge() {
    auto tmp = read<uint64_t>();
    if (flags == 1 || flags == 0) ip = tmp;
  };

  void ijle() {
    auto tmp = read<uint64_t>();
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
    auto org = read<uint64_t>();
    auto array = to_bin(regs.get(r1));
    for (uint8_t i = 0; i < size; i++) {
      memory[org+i] = array[i];
    }
    delete[] array;
  };

  void ixint() {
    auto index = read();
    if (index >= SYSCALL_COUNT) {
      throw std::out_of_range("Invalid syscall index\nThread: " + std::to_string(thread_id));
    }
    (this->*syscalls[index])();
  };

  void nop() {};

  void ilea() {
    auto r1 = read();
    auto ad = read<uint64_t>();

    if (ad >= SYSTEM_SEGMENT_START && !is_system) 
      throw std::runtime_error("Permission denied: Accessing system's segment.\n"
      "Thread: " + std::to_string(thread_id));
    
    if (ad >= HARDWARE_SEGMENT_START) {
      while (segments::keyboard_reserved);
      segments::keyboard_reserved = true;
    }
    
    regs.set(r1, ad);

    if (ad >= HARDWARE_SEGMENT_START) segments::keyboard_reserved = false;
  }

  void iloadat() {
    auto dst = read();
    auto at = read<uint64_t>(); /* This function will at memory[at]. */
    
    if (at >= SYSTEM_SEGMENT_START && !is_system) 
    throw std::runtime_error("Permission denied: Accessing system's segment.\n"
    "Thread: " + std::to_string(thread_id));
  
    if (at >= HARDWARE_SEGMENT_START) {
      while (segments::keyboard_reserved);
      segments::keyboard_reserved = true;
    }
    
    regs.set(dst, memory[at]);

    if (at >= HARDWARE_SEGMENT_START) segments::keyboard_reserved = false;
  }

  void iret() { ip = regs.get(63); }

  void imovad() {
    auto a = read(), b = read();
    regs.set(a, memory[regs.get(b)]);
  }

  setfunc_t set[22];

  void swritec() {
    std::putchar((char)regs.get(0));
  }
  
  void swritec_stderr() {
    fputc(static_cast<char>(regs.get(0)), stderr);
    fflush(stderr);
  }

  void sreadc() {
    int c = getchar();
    if (c != EOF) {
      regs.set(0, static_cast<uint8_t>(c));
    } else {
      regs.set(0, 0);
    }
  }

  void scsystem() {
    auto beg = regs.get(48);
    auto len = regs.get(49);

    if (beg + len >= SYSTEM_SEGMENT_START && !is_system) 
      throw std::runtime_error("Permission denied: Accessing system's segment.\n"
        "Thread: " + std::to_string(thread_id)); 

    if (beg + len >= HARDWARE_SEGMENT_START) {
      // Unlock the segment.
      while (segments::keyboard_reserved) ;
      segments::keyboard_reserved = true;
    } else if (beg + len >= end || beg + len <= beg) 
      throw std::runtime_error("Permission denied: Reading out of local segment.\n"
        "Thread: " + std::to_string(thread_id));
    
    std::string buff;
    for (uint64_t i = 0; i < len; i++) 
      buff += (char)memory[beg + i];
    
    regs.set(50, std::system(format(buff).c_str()));
  }

  void sldlib() {
    /* Calling a C-external function.
     If the DynamicLibrary class throws an error, 
     it's the process that will handle this error.
     However, if it's the extern C function that throw the error, 
     in the first case we need to handle this error into this function, 
     and then throw a C++ Exception. */

    std::string lib = "";
    // The pointer is stored in the 48 register.
    auto mybeg = regs.get(48);
    auto len = regs.get(49);

    if (mybeg < beg || mybeg > end) throw std::out_of_range("syscall call C: pointer out of range: " + std::to_string(mybeg));
    if (mybeg + len > end) throw std::out_of_range("Too large string (out of segment): " + std::to_string(mybeg+len));
    
    for (uint64_t i = 0; i < len; i++) lib += memory[mybeg+i];
    
    libs[regs.get(50)].loadLibrary(lib.c_str());
  }
  
  void sldlcfun() {
    auto mybeg = regs.get(48);
    auto len = regs.get(49);
    auto lib = regs.get(50);

    std::string func = "";

    if (mybeg < beg || mybeg > end) throw std::out_of_range("syscall call C: pointer out of range: " + std::to_string(mybeg));
    if (mybeg + len > end) throw std::out_of_range("Too large string (out of segment): " + std::to_string(mybeg+len));
    
    for (uint64_t i = 0; i < len; i++) func += memory[mybeg+i];

    if (libs.find(lib) == libs.end()) {
      std::ostringstream oss;
      oss << "Null pointing dynamic librarie handle: 0x"
          << std::hex << std::uppercase << lib << std::endl
          << "\tThread:\t" << std::dec << thread_id 
          << "\n\tIP:\t\t" << ip 
          << "\n\tLIP:\t\t" << local_ip
          << "\n\tloading:\t" << func << std::endl;
      throw std::runtime_error(oss.str());
    }

    auto funcptr = libs[lib].loadFunction(func.c_str());
    funcs[reinterpret_cast<uint64_t>(funcptr)] = funcptr;
    regs.set(50, reinterpret_cast<uint64_t>(funcptr));
  }

  void suldlib() {
    auto libHandle = regs.get(48);
    
    if ((libs.find(libHandle)) == libs.end()) {
      std::ostringstream oss;
      oss << "Null pointing dynamic librarie handle: 0x"
          << std::hex << std::uppercase << libHandle << std::endl
          << "\tThread:\t" << std::dec << thread_id 
          << "\n\tIP:\t\t" << ip 
          << "\n\tLIP:\t\t" << local_ip << std::endl;
      throw std::runtime_error(oss.str());
    }

    libs[libHandle].unloadLibrary();
  }

  void scfun() {
    auto funHandle = regs.get(48);
    
    if ((funcs.find(funHandle)) == funcs.end()) {
      std::ostringstream oss;
      oss << "Null pointing function handle: 0x"
          << std::hex << std::uppercase << funHandle << std::endl
          << "\tThread:\t" << std::dec << thread_id 
          << "\n\tIP:\t\t" << ip 
          << "\n\tLIP:\t\t" << local_ip << std::endl;
      throw std::runtime_error(oss.str());
    }

    /* Create arguments. */
    while (segments::keyboard_reserved) ;
    segments::keyboard_reserved = true;

    libcallc::arg_t arg{};
    arg.keyboardstart = &memory[KEYBOARD_SEGMENT_START];
    arg.regspointer   = &regs;
    arg.segstart      = &memory[beg];
    arg.seglen        = end - beg;

    funcs[funHandle](&arg);

    segments::keyboard_reserved = false;
  }

  void sstartt() {
    auto beg = regs.get(48);
    auto end = regs.get(49);

    if (beg > end) 
      throw std::logic_error("Cannot create a new thread with given arguments."
        " 'beg' flag is bigger than 'end' flag.\nThread: " + std::to_string(thread_id));

    if (beg < HARDWARE_SEGMENT_START && end > HARDWARE_SEGMENT_START)
      throw std::runtime_error("Thread memory region overlaps with HARDWARE_SEGMENT_START.\n"
        "Thread: " + std::to_string(thread_id));
      
    corewtarg64* c = new corewtarg64();
    manager::create_region(beg, end);
    c->init(beg, end, false, end+1);

    std::thread thread([this, c]() mutable {
      {
        std::lock_guard<std::mutex> lock(mtx);
        children++;
      }
      c->run();
      delete c;
      {
        std::lock_guard<std::mutex> lock(mtx);
        children--;
        if (children == 0) {
          cv.notify_all();
        }
      }
    });

    thread.detach();    
  }

  void spseg() {
    auto beg = regs.get(48);
    auto len = regs.get(49);
    auto org = regs.get(50);
    
    if (manager::is_region_created(org)) 
      throw std::runtime_error("Operating on an assigned region: " + std::to_string(org));

    for (uint64_t i = 0; i < len; i++) 
      memory[org+i] = memory[beg+i];
  }

  void sreads() {
    std::string buff;
    std::getline(std::cin, buff);

    while (segments::keyboard_reserved); /* Wait ! */
    segments::keyboard_reserved = true;

    size_t i = 0;

    while (i < buff.size() && KEYBOARD_SEGMENT_START+i < KEYBOARD_SEGMENT_END) { 
      memory[KEYBOARD_SEGMENT_START+i] = buff[i];
      i++;
    }

    segments::keyboard_reserved = false;
    regs.set(50, buff.size());
  }

  syscall_t syscalls[SYSCALL_COUNT] = {
    &corewtarg64::swritec,
    &corewtarg64::swritec_stderr,
    &corewtarg64::sreadc,
    &corewtarg64::scsystem,
    &corewtarg64::sldlib,
    &corewtarg64::sstartt,
    &corewtarg64::spseg,
    &corewtarg64::sreads, 
    /* New */
    &corewtarg64::sldlcfun, 
    &corewtarg64::suldlib,
    &corewtarg64::scfun,
  };

public:
  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system = false, 
            uint64_t _name = '?') override {
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    ip  = beg;
    is_system = _is_system;
    thread_id = _name;

    set[set_wtarg64::nop] = &corewtarg64::nop;
    set[set_wtarg64::lea] = &corewtarg64::ilea; 
    set[set_wtarg64::load] = &corewtarg64::iload;
    set[set_wtarg64::store] = &corewtarg64::istore;
    set[set_wtarg64::mov] = &corewtarg64::imov;
    set[set_wtarg64::add] = &corewtarg64::iadd;
    set[set_wtarg64::sub] = &corewtarg64::isub;
    set[set_wtarg64::mul] = &corewtarg64::imul;
    set[set_wtarg64::odiv] = &corewtarg64::idiv;
    set[set_wtarg64::mod] = &corewtarg64::imov;
    set[set_wtarg64::jmp] = &corewtarg64::ijmp;
    set[set_wtarg64::je] = &corewtarg64::ije;
    set[set_wtarg64::jne] = &corewtarg64::ijne;
    set[set_wtarg64::jl] = &corewtarg64::ijl;
    set[set_wtarg64::jg] = &corewtarg64::ijg;
    set[set_wtarg64::jle] = &corewtarg64::ijle;
    set[set_wtarg64::jge] = &corewtarg64::ijge;
    set[set_wtarg64::cmp] = &corewtarg64::icmp;
    set[set_wtarg64::xint] = &corewtarg64::ixint;
    set[set_wtarg64::loadat] = &corewtarg64::iloadat;
    set[set_wtarg64::ret]    = &corewtarg64::iret;
    set[set_wtarg64::movad]  = &corewtarg64::imovad;
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

      wyland_uint key = get_key();
      if (key)  regs.set(REG_KEY, key);
      
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