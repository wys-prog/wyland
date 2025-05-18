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

class wArchx87_128Exception : runtime::wyland_runtime_error {
private:
public:

  wArchx87_128Exception(const std::string &what, const std::string &from, uint64_t ip = 0, uint64_t thread_id = 0)
    : runtime::wyland_runtime_error(what.c_str(), "wArch x87-AZ8 Exception", from.c_str(), typeid(this).name(), ip, thread_id, nullptr, nullptr, 0)
  {}
};

namespace ProgramTracer {
  class ProgramTracer {
  private:
    std::stack<std::string> callstack;

  public:
    //! TODO: Adding tracing, with a map:
    //! 1 -> read() ? (EXMPL)
    //! && 
    //! tracer.size() ~= input.size() ? /!\: loops !!

    void TraceFunction(const std::string &name, const std::vector<std::string> &args) {
      auto now = std::chrono::high_resolution_clock::now();
      auto epoch = now.time_since_epoch();
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count() % 1000000000;
      std::ostringstream oss;
      oss << "**" << seconds << "." << std::setw(9) << std::setfill('0') << nanoseconds << ": Traced function:\t" << name;
      for (const auto &arg : args) oss << "\t" << arg;
      oss << std::endl;
      callstack.push(oss.str());
    }

    void TraceFunctionCalling(const std::string &name, const std::vector<std::string> &args) {
      auto now = std::chrono::high_resolution_clock::now();
      auto epoch = now.time_since_epoch();
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count() % 1000000000;
      std::ostringstream oss;
      oss << "***" << seconds << "." << std::setw(9) << std::setfill('0') << nanoseconds << ": (called) Traced function:\t" << name;
      for (const auto &arg : args) oss << "\t" << arg;
      oss << std::endl;
      callstack.push(oss.str());
    }

    void TraceGoBack() {
      auto now = std::chrono::high_resolution_clock::now();
      auto epoch = now.time_since_epoch();
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count() % 1000000000;
      std::ostringstream oss;
      oss << "***" << seconds << "." << std::setw(9) << std::setfill('0') << nanoseconds << ": (callback): returning to caller" << std::endl;
      callstack.push(oss.str());
    }

    void TraceGoBack(const std::string &from, const std::string &to) {
      auto now = std::chrono::high_resolution_clock::now();
      auto epoch = now.time_since_epoch();
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count() % 1000000000;
      std::ostringstream oss;
      oss << "***" << seconds << "." << std::setw(9) << std::setfill('0') << nanoseconds 
      << ": (callback): returning from " << from << " to " << to << std::endl;
      callstack.push(oss.str());
    }

    std::string GetStringStack() {
      std::ostringstream oss;
      std::stack<std::string> temp = callstack;  // Preserve original stack
      while (!temp.empty()) {
        oss << temp.top() << std::endl;
        temp.pop();
      }
      return oss.str();
    }

    void WriteToStream(std::ostream &oss) {
      std::stack<std::string> temp = callstack;
      while (!temp.empty()) {
        oss << temp.top() << std::endl;
        temp.pop();
      }
    }

    void WriteToStream(std::vector<std::ostream*> &vec) {
      std::stack<std::string> temp = callstack;
      while (!temp.empty()) {
        for (auto &stream : vec) ((*stream) << temp.top() << std::endl);
        temp.pop();
      }
    }

    void InitStack() {
      auto now = std::chrono::high_resolution_clock::now();
      auto epoch = now.time_since_epoch();
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
      auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count() % 1000000000;
      std::ostringstream oss;
      oss << "**Tracing started at " << seconds << "." << std::setw(9) << std::setfill('0') << nanoseconds << "**" << std::endl;
      callstack.push(oss.str());
    }
  };

  class TracerAgent {
  private:
    std::string Target;
    std::string To;
    ProgramTracer &TracerRef;

  public:
    TracerAgent() = delete;
    TracerAgent(
      const std::string &mcaller,
      const std::string &name, 
      const std::vector<std::string> &args, 
      ProgramTracer &Tracer) 
      : Target(name), To(mcaller), TracerRef(Tracer) {
      TracerRef.TraceFunctionCalling(name, args);
    }

    ~TracerAgent() {
      TracerRef.TraceGoBack(Target, To);
    }
  };
}

class warch128_backend : public wyland_base_core {
protected:
  class _wArch128RuntimeThrowInstance : public runtime::wyland_runtime_error {
  private:
    ProgramTracer::ProgramTracer &ProgramTracerRef;
  public:
    _wArch128RuntimeThrowInstance(const std::string &what, const std::string &from, uint64_t ip, uint64_t thread_id, ProgramTracer::ProgramTracer &PT)
      : runtime::wyland_runtime_error(what.c_str(), "wArch128RuntimeThrowInstance", from.c_str(), typeid(this).name(), ip, thread_id, nullptr, nullptr, 0), 
      ProgramTracerRef(PT)
    {
      PT.TraceFunction(
        memberofstr, {
          "-- Object of type '" + std::string(typeid(this).name()) + "' created (likely due to an exception being thrown by user or system code)",
          "-- Exception (probably) thrown manually, with message: " + what,
          "-- This class is generally used for exceptions defined in disk files that cause execution to abort."
        }
      );
    }

    ~_wArch128RuntimeThrowInstance() {
      ProgramTracerRef.TraceFunction(memberofstr, {"-- Object of type '" + std::string(typeid(this).name()) + "' destroyed (probably caught)"});
    }
  };

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
  BIOS    *Bios;
  ProgramTracer::ProgramTracer Tracer;
  wArch128Registers regs;

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

  void _exec_loop(const std::string &caller) {
    ProgramTracer::TracerAgent Agent(caller, memberofstr, {}, Tracer);
    while (!halted) {
      _exec_ins(read());
    }
  }

  void _exec_ins(uint8_t op) {
    switch (op) {
      break;
      case 0x01: imov(); break;
      case 0x02: iadd(); break;
      case 0x03: isub(); break;
      case 0x04: imul(); break;
      case 0x05: idiv(); break;
      case 0x06: imod(); break;
      case 0x07: icmp(); break;
      case 0x08: ijmp(); break;
      case 0x09: ije(); break;
      case 0x0A: jne(); break;
      case 0x0B: ijl(); break;
      case 0x0C: ijle(); break;
      case 0x0D: ijg(); break;
      case 0x0E: ijge(); break;
      case 0x0F: iijmprgs(); break;
      case 0x10: ijergs(); break;
      case 0x11: ijnergs(); break;
      case 0x12: ijlrgs(); break;
      case 0x13: ijlergs(); break;
      case 0x14: ijgrgs(); break;
      case 0x15: ijgergs(); break;
      case 0x16: ithrow(); break;
      case 0x17: iint(); break;
      case 0x18: imovIS(); break;
      case 0x19: iaddIS(); break;
      case 0x1A: isubIS(); break;
      case 0x1B: imulIS(); break;
      case 0x1C: idivIS(); break;
      case 0x1D: imodIS(); break;
      case 0x1E: isal(); break;
      case 0x1F: isar(); break;
      case 0x20: isalIS(); break;
      case 0x21: isarIS(); break;
      case 0x22: idec(); break;
      case 0x23: iinc(); break;
      case 0x24: iand(); break;
      case 0x25: ixor(); break;
      case 0x26: ior(); break;
      case 0x27: iandIS(); break;
      case 0x28: ixorIS(); break;
      case 0x29: iorIS(); break;
      case 0x2A: i_call_linked_function(); break;
      case 0xFE: break;
      case 0xFF: halted = true; break;
      default: 
        throw wArchx87_128Exception("Invalid Instruction Execution", memberofstr, ip, thread_id);
        break;
    }
  }

  template <typename T>
  void set_flags(const T &A, const T &B) {
    if (A == B) flags = EQUAL;
    else if (A > B) flags = LARGER;
    else flags = LESSER;
  }

#pragma region handles

  void imov() {
    auto size = read();
    switch (size) {
      case 8:   regs.set8(read(), read()); break; 
      case 16:  regs.set16(read(), read()); break; 
      case 32:  regs.set32(read(), read()); break; 
      case 64:  regs.set64(read(), read()); break; 
      case 128: regs.set128(read(), read()); break; 
      default:
        throw wArchx87_128Exception("Invalid Mov Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void iadd() {
    auto size = read();
    auto a = read();
    auto b = read();

    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) + regs.get8(b)); break; 
      case 16:  regs.set16(a, regs.get16(a) + regs.get16(b)); break; 
      case 32:  regs.set32(a, regs.get32(a) + regs.get32(b)); break; 
      case 64:  regs.set64(a, regs.get64(a) + regs.get64(b)); break; 
      case 128: regs.set128(a, regs.get128(a) + regs.get128(b)); break; 
      default:
        throw wArchx87_128Exception("Invalid Add Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void isub() {
    auto size = read();
    auto a = read();
    auto b = read();

    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) - regs.get8(b)); break; 
      case 16:  regs.set16(a, regs.get16(a) - regs.get16(b)); break; 
      case 32:  regs.set32(a, regs.get32(a) - regs.get32(b)); break; 
      case 64:  regs.set64(a, regs.get64(a) - regs.get64(b)); break; 
      case 128: regs.set128(a, regs.get128(a) - regs.get128(b)); break; 
      default:
        throw wArchx87_128Exception("Invalid Sub Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void imul() {
    auto size = read();
    auto a = read();
    auto b = read();

    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) * regs.get8(b)); break; 
      case 16:  regs.set16(a, regs.get16(a) * regs.get16(b)); break; 
      case 32:  regs.set32(a, regs.get32(a) * regs.get32(b)); break; 
      case 64:  regs.set64(a, regs.get64(a) * regs.get64(b)); break; 
      case 128: regs.set128(a, regs.get128(a) * regs.get128(b)); break; 
      default:
        throw wArchx87_128Exception("Invalid Mul Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void idiv() {
    auto size = read();
    auto a = read();
    auto b = read();

    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) / regs.get8(b)); break; 
      case 16:  regs.set16(a, regs.get16(a) / regs.get16(b)); break; 
      case 32:  regs.set32(a, regs.get32(a) / regs.get32(b)); break; 
      case 64:  regs.set64(a, regs.get64(a) / regs.get64(b)); break; 
      case 128: regs.set128(a, regs.get128(a) / regs.get128(b)); break; 
      default:
        throw wArchx87_128Exception("Invalid Div Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void imod() {
    auto size = read();
    auto a = read();
    auto b = read();

    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) % regs.get8(b)); break;
      case 16:  regs.set16(a, regs.get16(a) % regs.get16(b)); break;
      case 32:  regs.set32(a, regs.get32(a) % regs.get32(b)); break;
      case 64:  regs.set64(a, regs.get64(a) % regs.get64(b)); break;
      case 128: regs.set128(a, regs.get128(a) % regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid Mod Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void icmp() {
    auto size = read();
    auto a = read();
    auto b = read();

    switch (size) {
      case 8:   set_flags(regs.get8(a), regs.get8(b)); break;
      case 16:  set_flags(regs.get16(a), regs.get16(b)); break;
      case 32:  set_flags(regs.get32(a), regs.get32(b)); break;
      case 64:  set_flags(regs.get64(a), regs.get64(b)); break;
      case 128: set_flags(regs.get128(a), regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid (cmp)() Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void ijmp() {
    // Unconditional jump: set ip to the immediate 64-bit address
    ip = base_address + read<uint64_t>() - disk_base;
  }

  void ije() {
    // Jump if equal
    uint64_t target = base_address + read<uint64_t>() - disk_base;
    if (flags == EQUAL) ip = target;
  }

  void jne() {
    // Jump if not equal
    uint64_t target = base_address + read<uint64_t>() - disk_base;
    if (flags != EQUAL) ip = target;
  }

  void ijl() {
    // Jump if less
    uint64_t target = base_address + read<uint64_t>() - disk_base;
    if (flags == LESSER) ip = target;
  }

  void ijle() {
    // Jump if less or equal
    uint64_t target = base_address + read<uint64_t>() - disk_base;
    if (flags == LESSER || flags == EQUAL) ip = target;
  }

  void ijg() {
    // Jump if greater
    uint64_t target = read<uint64_t>();
    if (flags == LARGER) ip = target;
  }

  void ijge() {
    // Jump if greater or equal
    uint64_t target = base_address + read<uint64_t>() - disk_base;
    if (flags == LARGER || flags == EQUAL) ip = target;
  }

  void iijmprgs() {
    // Unconditional ijump: set ip to the value of the given register (64-bit)
    ip = base_address + regs.get64(read()) - disk_base;
  }

  void ijergs() {
    // Jump if equal to the value of the given register
    if (flags == EQUAL) ip = base_address + regs.get64(read()) - disk_base;
  }

  void ijnergs() {
    // Jump if not equal to the value of the given register
    if (flags != EQUAL) ip = base_address + regs.get64(read()) - disk_base;
  }

  void ijlrgs() {
    // Jump if less to the value of the given register
    if (flags == LESSER) ip = base_address + regs.get64(read()) - disk_base;
  }

  void ijlergs() {
    // Jump if less or equal to the value of the given register
    if (flags == LESSER || flags == EQUAL) ip = base_address + regs.get64(read()) - disk_base;
  }

  void ijgrgs() {
    // Jump if greater to the value of the given register
    if (flags == LARGER) ip = base_address + regs.get64(read()) - disk_base;
  }

  void ijgergs() {
    // Jump if greater or equal to the value of the given register
    if (flags == LARGER || flags == EQUAL) ip = base_address + regs.get64(read()) - disk_base;
  }

  void ithrow() {
    std::string what;
    while (auto i = read()) {
      what.push_back(i);
    }

    throw _wArch128RuntimeThrowInstance(what, memberofstr, ip, thread_id, Tracer);
  }

  void iint() {
    auto ptr = regs.swap();
    (*Bios).interrupt(read<wint>(), &ptr);
  }

  void imovIS() { // IV = Inline Source
    auto size = read();
    switch (size) {
      case 8: regs.set8(read(), read()); break;
      case 16: regs.set16(read(), read<wui16>()); break;
      case 32: regs.set32(read(), read<wui32>()); break;
      case 64: regs.set64(read(), read<wui64>()); break;
      case 128: regs.set128(read(), read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid MovIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void iaddIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) + read()); break;
      case 16:  regs.set16(a, regs.get16(a) + read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) + read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) + read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) + read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid AddIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void isubIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) - read()); break;
      case 16:  regs.set16(a, regs.get16(a) - read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) - read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) - read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) - read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid SubIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void imulIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) * read()); break;
      case 16:  regs.set16(a, regs.get16(a) * read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) * read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) * read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) * read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid MulIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void idivIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) / read()); break;
      case 16:  regs.set16(a, regs.get16(a) / read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) / read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) / read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) / read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid DivIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void imodIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) % read()); break;
      case 16:  regs.set16(a, regs.get16(a) % read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) % read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) % read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) % read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid ModIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void isal() {
    auto size = read();
    auto a = read();
    auto b = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) << regs.get8(b)); break;
      case 16:  regs.set16(a, regs.get16(a) << regs.get16(b)); break;
      case 32:  regs.set32(a, regs.get32(a) << regs.get32(b)); break;
      case 64:  regs.set64(a, regs.get64(a) << regs.get64(b)); break;
      case 128: regs.set128(a, regs.get128(a) << regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid Sal Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void isar() {
    auto size = read();
    auto a = read();
    auto b = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) >> regs.get8(b)); break;
      case 16:  regs.set16(a, regs.get16(a) >> regs.get16(b)); break;
      case 32:  regs.set32(a, regs.get32(a) >> regs.get32(b)); break;
      case 64:  regs.set64(a, regs.get64(a) >> regs.get64(b)); break;
      case 128: regs.set128(a, regs.get128(a) >> regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid Sar Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void isalIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) << read()); break;
      case 16:  regs.set16(a, regs.get16(a) << read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) << read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) << read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) << read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid SalIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void isarIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) >> read()); break;
      case 16:  regs.set16(a, regs.get16(a) >> read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) >> read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) >> read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) >> read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid SarIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void idec() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) - 1); break;
      case 16:  regs.set16(a, regs.get16(a) - 1); break;
      case 32:  regs.set32(a, regs.get32(a) - 1); break;
      case 64:  regs.set64(a, regs.get64(a) - 1); break;
      case 128: regs.set128(a, regs.get128(a) - 1); break;
      default:
        throw wArchx87_128Exception("Invalid Dec Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void iinc() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) + 1); break;
      case 16:  regs.set16(a, regs.get16(a) + 1); break;
      case 32:  regs.set32(a, regs.get32(a) + 1); break;
      case 64:  regs.set64(a, regs.get64(a) + 1); break;
      case 128: regs.set128(a, regs.get128(a) + 1); break;
      default:
        throw wArchx87_128Exception("Invalid Inc Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void iand() {
    auto size = read();
    auto a = read();
    auto b = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) & regs.get8(b)); break;
      case 16:  regs.set16(a, regs.get16(a) & regs.get16(b)); break;
      case 32:  regs.set32(a, regs.get32(a) & regs.get32(b)); break;
      case 64:  regs.set64(a, regs.get64(a) & regs.get64(b)); break;
      case 128: regs.set128(a, regs.get128(a) & regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid And Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void ixor() {
    auto size = read();
    auto a = read();
    auto b = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) ^ regs.get8(b)); break;
      case 16:  regs.set16(a, regs.get16(a) ^ regs.get16(b)); break;
      case 32:  regs.set32(a, regs.get32(a) ^ regs.get32(b)); break;
      case 64:  regs.set64(a, regs.get64(a) ^ regs.get64(b)); break;
      case 128: regs.set128(a, regs.get128(a) ^ regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid Xor Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void ior() {
    auto size = read();
    auto a = read();
    auto b = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) | regs.get8(b)); break;
      case 16:  regs.set16(a, regs.get16(a) | regs.get16(b)); break;
      case 32:  regs.set32(a, regs.get32(a) | regs.get32(b)); break;
      case 64:  regs.set64(a, regs.get64(a) | regs.get64(b)); break;
      case 128: regs.set128(a, regs.get128(a) | regs.get128(b)); break;
      default:
        throw wArchx87_128Exception("Invalid Or Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void iandIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) & read()); break;
      case 16:  regs.set16(a, regs.get16(a) & read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) & read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) & read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) & read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid AndIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void ixorIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) ^ read()); break;
      case 16:  regs.set16(a, regs.get16(a) ^ read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) ^ read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) ^ read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) ^ read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid XorIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void iorIS() {
    auto size = read();
    auto a = read();
    switch (size) {
      case 8:   regs.set8(a, regs.get8(a) | read()); break;
      case 16:  regs.set16(a, regs.get16(a) | read<wui16>()); break;
      case 32:  regs.set32(a, regs.get32(a) | read<wui32>()); break;
      case 64:  regs.set64(a, regs.get64(a) | read<wui64>()); break;
      case 128: regs.set128(a, regs.get128(a) | read<wui128>()); break;
      default:
        throw wArchx87_128Exception("Invalid OrIS Instruction Size", memberofstr, ip, thread_id);
        break;
    }
  }

  void i_call_linked_function() {
    // Call a linked function by index (function pointer table)
    auto func_index = read<uint32_t>();
    if (!linked_functions || linked_functions->find(func_index) == linked_functions->end()) {
      throw wArchx87_128Exception("Linked function index not found", memberofstr, ip, thread_id);
    }
    auto &func = (*linked_functions)[func_index];
    // You may want to pass arguments from registers or stack, depending on your ABI
    auto ptr = regs.swap();
    arg_t args {
      .segstart = &memory[beg], 
      .seglen = end - beg, 
      .keyboardstart = nullptr, 
      .regspointer = &ptr,
      .ip = ip, 
      .thread = thread_id, 
      .memory_start = &memory[0], 
      .is_system = (wbool)is_system
    };

    func(&args);
  }

  #warning "TODO: lea, etc.?"

#pragma endregion

  void init_table(linkedfn_array *table, const std::string &caller) {
    ProgramTracer::TracerAgent Agent(caller, memberofstr, {}, Tracer);

    if (!table) {
      linked_functions = new linkedfn_array();
    } else {
      linked_functions = table;
      std::cout << "[i]: " << linked_functions->size() << " extern function linked with the core" << std::endl;
    }
  }

  void init_modules(const std::vector<WylandMMIOModule*> &modules, const std::string &caller) {
    ProgramTracer::TracerAgent Agent(caller, memberofstr, {}, Tracer);

    security::SecurityAddModules(modules);
  }

  void _run() {
    auto start_time =  std::chrono::high_resolution_clock::now();
    
    _exec_loop(memberofstr);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long double> elapsed = end_time - start_time;
    std::ostringstream oss("");

    __wyland_long_float ips = local_ip / elapsed.count();
    oss << "Executed instructions: " << local_ip << "\n";
    oss << "Elapsed time: " << std::fixed << std::setprecision(8) << elapsed.count() << " seconds\n";
    oss << "VM speed: " << std::fixed << std::setprecision(2) << ips << " instructions/second\n";
    /*if (GlobalSettings::print_specs)*/ std::cout << oss.str() << std::flush;

    Tracer.TraceGoBack(memberofstr, "<@main> (unknown function)");
    Tracer.WriteToStream(std::cout);
  }

public:
  void run() override {
    Tracer.TraceFunction(
      memberofstr, {
        "-- Starting execution of core: " + memberofstr + " --"
      }
    );
    
    try {
      _run();
    } catch(const std::exception& e) {
      Tracer.TraceFunction(memberofstr, {"-- exception"});
      Tracer.WriteToStream(std::cout);
      throw;
    } catch (const runtime::wyland_runtime_error &e) {
      Tracer.TraceFunction(memberofstr, {"-- exception"});
      Tracer.WriteToStream(std::cout);
      throw;
    }
    
  }

  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system, 
            uint64_t _name, 
            linkedfn_array *table, 
            uint64_t _disk_relative, IWylandGraphicsModule *_GraphicsModule, 
            WylandMMIOModule *m1, WylandMMIOModule *m2, 
            WylandMMIOModule *disk, BIOS *BiosPtr) override {
    Tracer.InitStack();
    Tracer.TraceFunction(memberofstr, {});
    
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    is_system = _is_system;
    base_address = beg;
    disk_base = _disk_relative;
    thread_id = _name;
    
    init_table(table, memberofstr);
    if (!_GraphicsModule) _GraphicsModule = new IWylandGraphicsModule();
    if (!m1) m1 = new WylandMMIOModule();
    if (!m2) m2 = new WylandMMIOModule();
    if (!disk) disk = new WylandMMIOModule();
    
    init_modules({_GraphicsModule, m1, m2, disk}, memberofstr);
    BiosPtr->init({_GraphicsModule, m1, m2, disk}, _GraphicsModule, &memory[0], disk, cache::USBDevices);
    Tracer.TraceGoBack(memberofstr, "<@main> (unknown function)");
  }
};

ARCH_END
ARCH_END


#ifndef ___WYLAND_USE_NOT_V1___
typedef wylma::wyland::arch_x87_128::V1::warch128_backend core_warch128;
#endif // ___WYLAND_USE_NOT_V1___


WYLAND_END