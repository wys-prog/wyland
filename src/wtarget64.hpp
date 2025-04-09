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

#include <boost/container/flat_map.hpp>

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

#include "interfaces/interface.hpp"
#include "wmmio.hpp"

WYLAND_BEGIN

class corewtarg64 : public core_base {
  using syscall_t = void(corewtarg64::*)();
  using setfunc_t = void(corewtarg64::*)();

protected:
  uint64_t beg = 0x0000000000000000;
  uint64_t end = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip  = 0x0000000000000000;
  uint64_t local_ip = 0x0000000000000000;
  uint64_t base_address = 0x0000000000000000;
  reg_t    regs;
  bool     halted = false;
  int      flags  = 0;
  bool     is_system = false;
  uint64_t thread_id = '?' * '?';
  uint8_t  children = 0;
  std::mutex mtx;
  std::condition_variable cv;
  IWylandGraphicsModule *GraphicsModule;
  WylandMMIOModule      *MMIOModule1;
  WylandMMIOModule      *MMIOModule2;

  /* Deprecated ! */
  std::unordered_map<uint64_t, libcallc::DynamicLibrary> libs;
  std::unordered_map<uint64_t, libcallc::DynamicLibrary::FunctionType> funcs;

  boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> *linked_functions;
  
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
    if (flags == EQUAL) ip = tmp;
  };

  void ijne() {
    auto tmp = read<uint64_t>();
    if (flags == EQUAL) return;
    ip = tmp;
  };

  void ijg() {
    auto tmp = read<uint64_t>();
    if (flags == LARGER) ip = tmp;
  };

  void ijl() {
    auto tmp = read<uint64_t>();
    if (flags == LESSER) ip = tmp;
  };

  void ijge() {
    auto tmp = read<uint64_t>();
    if (flags == LARGER || flags == EQUAL) ip = tmp;
  };

  void ijle() {
    auto tmp = read<uint64_t>();
    if (flags == LESSER || flags == EQUAL) ip = tmp;
  };

  void icmp() {
    auto r1 = regs.get(read());
    auto r2 = regs.get(read());
  
    if (r1 > r2) {
      flags = LARGER;
    } else if (r1 == r2) {
      flags = EQUAL;
    } else {
      flags = LESSER;
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
      + "\n\tARGS:    [" + std::to_string((int)size) + ", " + std::to_string((int)r1) + "]"); 
      break;
    }

  }

  void istore() {
    auto size = read() / 8;
    auto r1 = read();
    auto org = base_address + read<uint64_t>();
    auto array = to_bin(regs.get(r1));
    for (uint8_t i = 0; i < size; i++) {
      memory[org+i] = array[i];
    }
    delete[] array;
  };

  void ixint() {
    // TODO ?
  };

  void nop() {};

  void ilea() {
    auto r1 = read();
    auto ad = base_address + read<uint64_t>();

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
    auto at = base_address + read<uint64_t>(); /* This function will at memory[at]. */
    
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
    regs.set(a, memory[base_address + regs.get(b)]);
  }

  void isal() { /* New in std:wy2.3 ! */
    auto r1 = read();
    auto with = read<uint64_t>();
    regs.set(r1, regs.get(r1) << with);
  }

  void isar() { /* New in std:wy2.3 ! */
    auto r1 = read();
    auto with = read<uint64_t>();
    regs.set(r1, regs.get(r1) >> with);
  }

  void iwthrow() { /* New in std:wy2.3 ! */
    std::string what = "";
    uint8_t c = read();
    while (c != 0) {
      what += c;
      c = read();
    }

    throw runtime::wyland_runtime_error(
      strdup(what.c_str()), "wyland runtime error", "wyland:vm:iwthrow()", 
      typeid(runtime::wyland_runtime_error).name(), ip, 
      thread_id, (uint64_t*)&memory[beg], 
      (uint64_t*)&memory[end], end - beg
    );
  }

  void iclfn() { /* New in std:wy2.4 ! */
    uint32_t id = read<uint32_t>();
    arg_t arguments;
    auto wrapped = regs.wrap();
    arguments.regspointer = &wrapped;
    arguments.keyboardstart = &memory[KEYBOARD_SEGMENT_START];
    arguments.seglen = end - beg;
    arguments.segstart = &memory[beg];
    arguments.ip = ip;
    arguments.thread = thread_id;
    arguments.is_system = is_system;
    arguments.memory_start = &memory[0];

    if (linked_functions->find(id) != linked_functions->end()) {
      linked_functions->at(id)(&arguments);
    } else {
      std::stringstream error;
      error << "linked function not found.\n"
               "\t\tcalling function: " << id << " but not found"; 
      throw runtime::wyland_invalid_pointer_exception(error.str().c_str(), "invalid pointer", __func__, ip, thread_id, NULL, NULL, end - beg);
    }
  }

  void iemplace() { /* New in std:wy2.4 ! */
    uint64_t address = regs.get(read());
    uint8_t with = read();

    if (address <= beg || address >= end) throw std::out_of_range(
      "Reading out of the local segment.\n"
      "\tflag 'beg':\t" + std::to_string(beg) + "\n"
      "\tflag 'end':\t" + std::to_string(end) + "\n"
      "\tIP (global):\t" + std::to_string(ip) + "\n"
      "\tthread:\t\t" + std::to_string(thread_id)  + "\n"
      "\tlocal IP:\t" + std::to_string(local_ip) + "\n"
      "\tfrom wtarg64::iemplace()"
    );

    auto array = to_bin(regs.get(with));
    auto max = reg_t::get_len(with);
    for (uint8_t i = 0; i < max; i++) {
      memory[address + i] = array[7 - i];
    }
  }

  void ipushmmio() { /* New in std:wy2.5 ! */
    auto index = read();
    auto bytes = read<uint64_t>();
    switch (index) {
      case 0: GraphicsModule->send_data(bytes); break;
      case 1: MMIOModule1->send_data(bytes); break;
      case 2: MMIOModule2->send_data(bytes); break;
      default: 
        throw std::runtime_error("somewhere::ipushmmio(): given index is invalid (from 0 to 2 only is valid): " + std::to_string(index));
        break;
    }
  }

  void ipopmmio() { /* New in std:wy2.5 ! */
    auto index = read();
    switch (index) {
      case 0: regs.set(R_POP_MMIO, GraphicsModule->receive_data()); break; /* ... What would you like to recive from.. That ? */
      case 1: regs.set(R_POP_MMIO, MMIOModule1->receive_data()); break;
      case 2: regs.set(R_POP_MMIO, MMIOModule2->receive_data()); break;
      default: 
        throw std::runtime_error("somewhere::ipushmmio(): given index is invalid (from 0 to 2 only is valid): " + std::to_string(index));
        break;
    }
  }

  setfunc_t set[29];

public:
  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system, 
            uint64_t _name, 
            linkedfn_array *table, 
            uint64_t base, IWylandGraphicsModule *_GraphicsModule) override {
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    ip  = beg;
    is_system = _is_system;
    thread_id = _name;

    if (table == nullptr) {
      throw std::runtime_error("linked_functions table is null.");
    }

    if (_GraphicsModule == nullptr) {
      GraphicsModule = new IWylandGraphicsModule();
    } else GraphicsModule = _GraphicsModule;

    linked_functions = table;

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
    set[set_wtarg64::sal] = &corewtarg64::isal;
    set[set_wtarg64::sar] = &corewtarg64::isar;
    set[set_wtarg64::wthrow] = &corewtarg64::iwthrow;
    set[set_wtarg64::clfn] = &corewtarg64::iclfn;
    set[set_wtarg64::empl] = &corewtarg64::iemplace;
    set[set_wtarg64::push_mmio] = &corewtarg64::ipushmmio;
    set[set_wtarg64::pop_mmio] = &corewtarg64::ipopmmio;

    /* Initialize the core, with some "basic" values. */

    regs.set(R_ORG, beg);
    regs.set(R_STACK_BASE, end - STACK_SIZE);
    regs.set(0, 'A');
    regs.set(1, 1);
    regs.set(REG_KEY, 0x00);

    if (is_system) {
      if (!GraphicsModule->init(800, 600, "Wyland")) {
        std::cerr << "[e]: unable to initialize <GraphicsModule*>" << std::endl;
        throw GraphicsModuleException(
          "Unable to initialize <GraphicsModule*>", __func__ 
        );
      } else std::cout << "[i]: GraphicsModule initialized: " << GraphicsModule->name() << std::endl;
    }
  }

  void run() override {
    auto last = std::chrono::high_resolution_clock::now();
    while (!halted) {
      if (ip < beg || ip > end) 
      throw std::out_of_range(
        "Reading out of the local segment.\n"
        "\tflag 'beg':\t" + std::to_string(beg) + "\n"
        "\tflag 'end':\t" + std::to_string(end) + "\n"
        "\tIP (global):\t" + std::to_string(ip) + "\n"
        "\tthread:\t\t" + std::to_string(thread_id)  + "\n"
        "\tlocal IP:\t" + std::to_string(local_ip) + "\n"
        "\tfrom wtarg64::run()"
      );
      
      auto fetched = read();
      local_ip++;

      if (fetched == 0xFF) { halted = true; continue; }
      if (fetched == 0xFE) { continue; } /* Specific mark for labels. Used for debugging. */

      wyland_uint key = get_key();
      if (key)  {
        regs.set(REG_KEY, key); // What are you looking for ?
      }
      
      if (fetched >= sizeof(set) / sizeof(set[0])) {
        std::ostringstream oss;
        oss << "Invalid instruction: " << std::hex << std::uppercase << (int)fetched << "\n"
        "\tfetched:\t[" << (int)fetched<< "]\n"
        "\t4 bytes:\t[" <<  
        format({memory[ip-2],memory[ip-1],memory[ip],memory[ip+1]}, ',') 
        << "]\n"
        "\tCore IP:\t" << ip << std::dec << "\n"
        "\tLocal IP:\t" << local_ip << "\n\tThread:\t" << thread_id;
        throw std::runtime_error(oss.str());
      }

      try {
        (this->*set[fetched])();
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = now - last;
        GraphicsModule->process(float_to_wfloat(delta.count()));
        GraphicsModule->render();
        last = now;
        if (GraphicsModule->should_close()) halted = true;
      } catch (const std::exception &e) {
        throw runtime::wyland_runtime_error(e.what(), "Instruction Invokation Exception", __func__, typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      } catch (const runtime::wyland_runtime_error &e) {
        throw runtime::wyland_runtime_error(e.what(), e.name(), e.caller(), typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      }
    }

    GraphicsModule->shutdown();

    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this] { return children == 0; });
    }
  }

  uint64_t get_ip() override { return ip; }

  void run_step() override {
    if (!halted) {

      if (ip < beg || ip > end) 
      throw std::out_of_range(
        "Reading out of the local segment.\n"
        "\tflag 'beg':\t" + std::to_string(beg) + "\n"
        "\tflag 'end':\t" + std::to_string(end) + "\n"
        "\tIP (global):\t" + std::to_string(ip) + "\n"
        "\tthread:\t\t" + std::to_string(thread_id)  + "\n"
        "\tlocal IP:\t" + std::to_string(local_ip) + "\n"
        "\tfrom wtarg64::run()"
      );
      
      auto fetched = read();
      local_ip++;

      if (fetched == 0xFF) { halted = true; return; }
      if (fetched == 0xFE) { return; } /* Specific mark for labels. Used for debugging. */

      wyland_uint key = get_key();
      if (key)  {
        regs.set(REG_KEY, key);
      }
      
      if (fetched >= sizeof(set) / sizeof(set[0])) {
        std::ostringstream oss;
        oss << "Invalid instruction: " << std::hex << std::uppercase << (int)fetched << "\n"
        "\tfetched:\t[" << (int)fetched<< "]\n"
        "\t4 bytes:\t[" <<  
        format({memory[ip-2],memory[ip-1],memory[ip],memory[ip+1]}, ',') 
        << "]\n"
        "\tCore IP:\t" << ip << std::dec << "\n"
        "\tLocal IP:\t" << local_ip << "\n\tThread:\t" << thread_id;
        throw std::runtime_error(oss.str());
      }

      try {
        (this->*set[fetched])();
      } catch (const std::exception &e) {
        throw runtime::wyland_runtime_error(e.what(), "Instruction Invokation Exception", __func__, typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      } catch (const runtime::wyland_runtime_error &e) {
        throw runtime::wyland_runtime_error(e.what(), e.name(), e.caller(), typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      } 
    }
  }
};

WYLAND_END