#pragma once

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
#include <stack>
#include <new>

#include <boost/container/flat_map.hpp>
#include "libcallc.hpp"

#include "wyland-runtime/keys.h"
#include "wyland-runtime/wylrt.h"
#include "wyland-runtime/wylrt.hpp"

#include "targets.h"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wformat.hpp"
#include "wtargb.hpp"

#include "interfaces/interface.hpp"
#include "wmmio.hpp"
#include "security.hpp"
#include "bios/bios.hpp"
#include "bios/bios_usb.hpp"

#include "wc++std.hpp"

#ifdef ___WYLAND_GNU_USE_FLOAT128___
#include <quadmath.h>
#endif // ? ___WYLAND_GNU_USE_FLOAT128___

/* ================================= =================== ================================= */
/* ================================= Warch128's includes ================================= */
/* ================================= =================== ================================= */

#include "warch128_regs.hpp"

WYLAND_BEGIN

ARCH_BACK(arch_x87_128) 
ARCH_BACK_V(V1)

namespace ProgramTracer {
  class WylandArchx87_128ProgramTracer {
  private:
    std::stack<std::string> callstack;  

  public:
    void TraceFunction(const std::string &name, const std::vector<std::string> &args) {
      std::ostringstream oss("");
      oss << "**" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << ": Traced function:\t" << name;
      for (const auto&arg:args) oss << "\t" << arg << std::endl;
      callstack.push(name.c_str());
    }

    std::string GetStringStack() {
      std::ostringstream oss("");
      while (!callstack.empty()) {
        oss << callstack.top() << std::endl;
        callstack.pop();
      }

      return oss.str();
    }

    void InitStack() {
      callstack.push(
        "**Tracing started at " + std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) + "**"
      );
    }
  };
};

typedef wui16 fedins; // FEtcheD INStruction

class warch128_backend {
protected:
  uint64_t beg           = 0x0000000000000000;
  uint64_t end           = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip            = 0x0000000000000000;
  uint64_t local_ip      = 0x0000000000000000; /* PAY ATTENTION !! LOCAL IP ISN'T LIKE IP !! Simply, it counts executed instructions... */
  uint64_t base_address  = 0x0000000000000000;
  uint64_t disk_base     = 0x0000000000000000;
  bool     halted = false;
  wint     flags  = 0; // Big issue !! Don't use int here !!
  bool     is_system = false;
  uint64_t thread_id = std::chrono::high_resolution_clock::now().time_since_epoch().count(); // Wtf ?
  IWylandGraphicsModule *GraphicsModule;
  WylandMMIOModule      *MMIOModule1;
  WylandMMIOModule      *MMIOModule2;
  WylandMMIOModule      *DiskModule;
  WylandMMIOModule      *Modules[4];
  BIOS                  *Bios;

  boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> *linked_functions;
  
  inline uint8_t read() {
    if (ip + 1 > end) wthrow (std::out_of_range(memberofstr + "(): ""The 'end' flag is reached.\n"
        "\tThread:\t" + std::to_string(thread_id) + "\n\tfrom read<u8>()\n\tip:\t" + std::to_string(ip) + 
        "\n\t'end':\t" + std::to_string(end) + "\n"
        "\texecuted: " + std::to_string(local_ip)));
    return memory[ip++];
  }

  template <typename T>
  inline T read() {
    if (ip + sizeof(T) > end) wthrow(std::out_of_range(memberofstr + "(): ""End of segment reached.\n"
      "\tThread:\t" + std::to_string(thread_id) + "\n\tIP:\t" + std::to_string(ip) + 
      "\n\t'end':\t" + std::to_string(end) + "\n"));

    T value;
    std::memcpy(&value, &memory[ip], sizeof(T));
    value = correct_byte_order<T>(value);
    ip += sizeof(T);
    return value;
  }

  warch128_regs regs;

  void _exec_loop() {
    while (!halted) {

    }
  }

  void _exec_ins() {

  }

public:
};

ARCH_END
ARCH_END

WYLAND_END