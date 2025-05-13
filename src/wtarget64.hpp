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
#include "security.hpp"
#include "bios/bios.hpp"
#include "bios/bios_usb.hpp"

#include "wc++std.hpp"

#ifdef ___WYLAND_GNU_USE_FLOAT128___
#include <quadmath.h>
#endif // ? ___WYLAND_GNU_USE_FLOAT128___

WYLAND_BEGIN

class corewtarg64 : public core_base {
  using syscall_t = void(corewtarg64::*)();
  using setfunc_t = void(corewtarg64::*)();

protected:
  uint64_t beg           = 0x0000000000000000;
  uint64_t end           = 0xFFFFFFFFFFFFFFFF;
  uint64_t ip            = 0x0000000000000000;
  uint64_t local_ip      = 0x0000000000000000; /* PAY ATTENTION !! LOCAL IP ISN'T LIKE IP !! Simply, it counts executed instructions... */
  uint64_t base_address  = 0x0000000000000000;
  uint64_t disk_base     = 0x0000000000000000;
  reg_t    regs;
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
    if (ip + sizeof(T) > end) wthrow(std::out_of_range(memberofstr + "(): ""End of segment reached (read_value).\n"
      "\tThread:\t" + std::to_string(thread_id) + "\n\tIP:\t" + std::to_string(ip) + 
      "\n\t'end':\t" + std::to_string(end) + "\n"));

    T value;
    std::memcpy(&value, &memory[ip], sizeof(T));
    value = SWAP64(value);
    ip += sizeof(T);
    return value;
  }

  void imov() {
    auto r1 = read(), r2 = read();
    regs.set(r1, regs.get(r2)); 
  };

  void iadd() {
    auto r1 = read(), r2 = read();
    regs.set(r1, (regs.get(r1) + regs.get(r2)));
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
    ip = (base_address + read<uint64_t>()) - disk_base;
  };

  void ije() {
    auto tmp = base_address + read<uint64_t>() - disk_base;
    if (flags == EQUAL) ip = tmp;
  };

  void ijne() {
    auto tmp = base_address + read<uint64_t>() - disk_base;
    if (flags == EQUAL) return;
    ip = tmp;
  };

  void ijg() {
    auto tmp = base_address + read<uint64_t>() - disk_base;
    if (flags == LARGER) ip = tmp;
  };

  void ijl() {
    auto tmp = base_address + read<uint64_t>() - disk_base;
    if (flags == LESSER) ip = tmp;
  };

  void ijge() {
    auto tmp = base_address + read<uint64_t>() - disk_base;
    if (flags == LARGER || flags == EQUAL) ip = tmp;
  };

  void ijle() {
    auto tmp = base_address + read<uint64_t>() - disk_base;
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
      case 128: regs.set(r1, read<__uint128_t>(), 0); break;
      default: wthrow (std::invalid_argument("in iload(): Invalid size: " + std::to_string(size)
      + "\n\tfetched: [" + std::to_string((int)size) + "]"
        "\n\tThread:  " + std::to_string(thread_id) 
      + "\n\tIP:      " + std::to_string(ip)
      + "\n\tLIP:     " + std::to_string(local_ip)
      + "\n\tARGS:    [" + std::to_string((int)size) + ", " + std::to_string((int)r1) + "]")); 
      break;
    }
  }

  void istore() {
    auto size = read() / 8;
    auto r1 = read();
    auto org = base_address + read<uint64_t>() - disk_base;
    auto array = to_bin(regs.get(r1));
    for (uint8_t i = 0; i < size; i++) {
      memory[org+i] = array[i];
    }
    delete[] array;
  };

  void ixint() {
    auto regs_ptr =  regs.wrap();
    Bios->interrupt(read<wint>(), &regs_ptr);
  };

  void nop() {};

  void ilea() {
    auto r1 = read();
    auto ad = base_address + read<uint64_t>() - disk_base;

    if (ad > end || ad < beg) {
      wthrow std::out_of_range("Permission denied: out of local segment (from: " + memberofstr + ")");
    }
    
    regs.set(r1, ad);
  }
  
  void iloadat() {
    auto dst = read();
    auto at = base_address + read<uint64_t>() - disk_base; /* This function will at memory[at]. */
    //                                                              ^^^^^^^^^^^^^^^ wtf ?

    if (at > end || at < beg) {
      wthrow std::out_of_range("Permission denied: out of local segment (from: " + memberofstr + ")");
    }

    regs.set(dst, memory[at]);
  }

  void iret() { ip = regs.get(R_RET); }

  void imovad() {
    auto a = read(), b = read();
    regs.set(a, memory[base_address + regs.get(b) - disk_base]);
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
      STRDUP(what.c_str()), "wyland runtime error", "wyland/vm/iwthrow()", 
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
      wthrow (runtime::wyland_invalid_pointer_exception(error.str().c_str(), "invalid pointer", memberofcstr, ip, thread_id, NULL, NULL, end - beg));
    }
  }

  void iemplace() { /* New in std:wy2.4 ! */
    uint64_t address = regs.get(read()) - disk_base;
    uint8_t with = read();

    if (address <= beg || address >= end) wthrow (std::out_of_range(
      "Reading out of the local segment.\n"
      "\tflag 'beg':\t" + std::to_string(beg) + "\n"
      "\tflag 'end':\t" + std::to_string(end) + "\n"
      "\tIP (global):\t" + std::to_string(ip) + "\n"
      "\tthread:\t\t" + std::to_string(thread_id)  + "\n"
      "\tlocal IP:\t" + std::to_string(local_ip) + "\n"
      "\tfrom wtarg64::iemplace()")
    );

    auto array = to_bin(regs.get(with));
    auto max = reg_t::get_len(with);
    for (uint8_t i = 0; i < max; i++) {
      memory[address + i] = array[7 - i];
    }
  }

  void ipushmmio() { /* New in std:wy2.5 ! */
    auto index = read();
    auto reg = read();
    auto bytes = regs.get(reg);
    if (index >= 4) {
      wthrow (std::runtime_error("somewhere::ipushmmio(): given index is invalid (from 0 to 3 only is valid): " + std::to_string(index)));
    } else {
      Modules[index]->send_data(bytes);
    }
  }

  void ipopmmio() { /* New in std:wy2.5 ! */
    auto index = read();
    if (index >= 4) {
      wthrow (std::runtime_error(memberofstr + "(): given index is invalid (from 0 to 3 only is valid): " + std::to_string(index)));
    } else {
      regs.set(R_POP_MMIO, Modules[index]->receive_data());
    }
  }

  void iconnectmmio() { /* New in std:wy2.6 ! */
    auto index = read();
    auto namebeg = base_address + read<uint64_t>() - disk_base;
    uint64_t i = 0;
    int8_t c = (int8_t)memory[base_address + namebeg + (i)];
    std::string name = "";
    while (c) {
      name += c;
      c = (int8_t)memory[base_address + namebeg + (i++)];
    }

    if (index >= 4) {
      wthrow (std::runtime_error(memberofstr + "(): given index is invalid (from 0 to 3 only is valid): " + std::to_string(index)));
    } else {
      Modules[index] = loadIExternalMMIOModule(name);
    }
  }

  void ideconnectmmio() { /* New in std:wy2.6 ! */
    auto index = read();
    if (index >= 4) {
      wthrow (std::runtime_error(memberofstr + "(): ""given index is invalid (from 0 to 3 only is valid): " + std::to_string(index)));
    } else {
      Modules[index]->shutdown();
      Modules[index]->~WylandMMIOModule();
    }
  }

  void ixor() { /* New in std:wy2.6 ! */
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) ^ regs.get(r2));
  }

  void ior() { /* New in std:wy2.6 ! */
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) | regs.get(r2));
  }

  void iand() { /* New in std:wy2.6 ! */
    auto r1 = read(), r2 = read();
    regs.set(regs.get(r1), regs.get(r1) & regs.get(r2));
  }

  void iinc() { /* New in std:wy2.7 ! */
    auto r1 = read();
    regs.set((r1), regs.get(r1)+1);
  }

  void idec() { /* New in std:wy2.7 ! */
    auto r1 = read();
    regs.set((r1), regs.get(r1)+1);
  }

#ifndef ___WYLAND_SWITCH_INSTRUCTIONS___
  setfunc_t set[36];

  void init_set() {
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
    set[set_wtarg64::owthrow] = &corewtarg64::iwthrow;
    set[set_wtarg64::clfn] = &corewtarg64::iclfn;
    set[set_wtarg64::empl] = &corewtarg64::iemplace;
    set[set_wtarg64::push_mmio] = &corewtarg64::ipushmmio;
    set[set_wtarg64::pop_mmio] = &corewtarg64::ipopmmio;
    set[set_wtarg64::connect_mmio] = &corewtarg64::iconnectmmio;
    set[set_wtarg64::deconnect_mmio] = &corewtarg64::ideconnectmmio;
    set[set_wtarg64::oand] = &corewtarg64::iand;
    set[set_wtarg64::oor] = &corewtarg64::ior;
    set[set_wtarg64::oxor] = &corewtarg64::ixor;
    set[set_wtarg64::dec] = &corewtarg64::idec;
    set[set_wtarg64::inc] = &corewtarg64::iinc;
  }
#else
  void init_set() {}
#endif //___WYLAND_SWITCH_INSTRUCTIONS___

  void system_init() {
    // We check 'is_system' here, to "add security".
    if (is_system) {
      if (!GraphicsModule->init(800, 600, "Wyland")) {
        std::cerr << "[e]: unable to initialize <GraphicsModule*>" << std::endl;
        wthrow (GraphicsModuleException(
          "Unable to initialize <GraphicsModule*>", memberofcstr 
        ));
      } else std::cout << "[i]: GraphicsModule initialized: " << GraphicsModule->name() << std::endl;

      Bios->init({MMIOModule1, MMIOModule2, DiskModule}, GraphicsModule, memory, DiskModule, cache::USBDevices);

      security::SecurityAddModules({GraphicsModule, MMIOModule1, MMIOModule2, DiskModule});
      std::cout.clear();
    }
  }

  void registers_init() {
    regs.set(R_ORG, beg);
    regs.set(R_STACK_BASE, end - STACK_SIZE); // You can also put stack where ever you want...
    regs.set(0, 'A');
    regs.set(1, 1);
    regs.set(REG_KEY, 0x00);
    regs.set(R_MEMORY_INF, global::memory_size);
    regs.set(R_RELATIVE_N, base_address);
  }

public:
  void init(uint64_t _memory_segment_begin, 
            uint64_t _memory_segment_end, 
            bool _is_system, 
            uint64_t _name, 
            linkedfn_array *table, 
            uint64_t _disk_relative, IWylandGraphicsModule *_GraphicsModule, 
            WylandMMIOModule *m1, WylandMMIOModule *m2, 
            WylandMMIOModule *disk, BIOS *BiosPtr) override {
    beg = _memory_segment_begin;
    end = _memory_segment_end;
    ip  = beg;
    is_system = _is_system;
    thread_id = _name;
    base_address = _memory_segment_begin;
    disk_base = _disk_relative;
    Bios = BiosPtr;
    if (_is_system) {
      std::cout << "[i]: base disk address: 0x" << std::hex << std::setfill('0') << std::setw(16) << _disk_relative << std::endl;
      std::cout << "[i]: 'beg' flag: 0x" << std::hex << std::setfill('0') << std::setw(16) << beg << std::endl;
      std::cout << "[i]: 'end' flag: 0x" << std::hex << std::setfill('0') << std::setw(16) << end << std::endl;
      std::cout << "[i]: 'ip' flag: 0x" << std::hex << std::setfill('0') << std::setw(16) << ip << std::endl;
    }

    std::cout.clear();

    if (table == nullptr) {
      wthrow (std::runtime_error(memberofstr + "(): ""linked_functions table is null."));
    }

    if (_GraphicsModule == nullptr) {
      GraphicsModule = new IWylandGraphicsModule();
    } else GraphicsModule = _GraphicsModule;

    if (m1 == nullptr) {
      MMIOModule1 = new WylandMMIOModule();
    } else MMIOModule1 = m1;

    if (m2 == nullptr) {
      MMIOModule2 = new WylandMMIOModule();
    } else MMIOModule2 = m2;

    if (disk == nullptr) {
      DiskModule = new WylandMMIOModule();
    } else DiskModule = disk;

    linked_functions = table;
    Modules[0] = GraphicsModule;
    Modules[1] = DiskModule;
    Modules[2] = MMIOModule1;
    Modules[3] = MMIOModule2;

    init_set();
    /* Initialize the core, with some "basic" values. */

    system_init();
    registers_init();
  }
  
  void run() override {
    auto start_time =  std::chrono::high_resolution_clock::now();

    while (!halted) {
      // if (ip < beg || ip >= end) {}
      
      auto fetched = read();
      local_ip++;
#ifndef ___WYLAND_SWITCH_INSTRUCTIONS___
      if (fetched >= sizeof(set) / sizeof(set[0])) {
        if (fetched == 0xFF) { break; }
        if (fetched == 0xFE) { continue; } /* Specific mark for labels. Used for debugging. */

        std::ostringstream oss;
        oss << "Invalid instruction: " << std::hex << std::setw(2) << std::setfill('0') << (int)fetched << "\n"
        "\tfetched:\t[" << (int)fetched<< "]\n"
        "\t4 bytes:\t[" <<  
        format({memory[ip-2],memory[ip-1],memory[ip],memory[ip+1]}, ',') 
        << "]\n"
        "\tCore IP:\t0x" << std::setw(16) << std::setfill('0') << std::hex << ip  << "\n"
        "\tLocal IP:\t0x" << std::setw(16) << std::setfill('0') << std::hex << local_ip << "\n"
        "\tDisk IP:\t0x" << std::setw(16) << std::setfill('0') << std::hex << code_start + ip << "\n"
        "\tbase.add:\t0x" << std::setw(16) << std::setfill('0') << std::hex << base_address << "\n"
        "\trel.add:\t0x"  << std::setw(16) << std::setfill('0') << std::hex << code_start << "\n"
        "\tThread:\t" << std::dec << thread_id << "\n"
        "from: " << memberofstr + "(): ";
        wthrow (std::runtime_error(oss.str()));
      }

      try {
        (this->*set[fetched])();
      } catch (const std::exception &e) {
        show_specs(start_time);
        throw runtime::wyland_runtime_error(e.what(), "Instruction Invokation Exception", memberofcstr, typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      } catch (const runtime::wyland_runtime_error &e) {
        show_specs(start_time);
        throw runtime::wyland_runtime_error(e.what(), e.name(), e.caller(), typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      }
#else
    
      _switch_instruction(fetched);
      
#endif // ? ___WYLAND_SWITCH_INSTRUCTIONS___
    }

    GraphicsModule->shutdown();
    MMIOModule1->shutdown();
    MMIOModule2->shutdown();
    DiskModule->shutdown();
    show_specs(start_time);
  }

  void show_specs(const std::chrono::high_resolution_clock::time_point &start_time) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long double> elapsed = end_time - start_time;

    __wyland_long_float ips = local_ip / elapsed.count();
    std::cout << "Executed instructions: " << local_ip << "\n";
    std::cout << "Elapsed time: " << std::fixed << std::setprecision(8) << elapsed.count() << " seconds\n";
    std::cout << "VM speed: " << std::fixed << std::setprecision(2) << ips << " instructions/second\n";
  }

  uint64_t get_ip() override { return ip; }

#ifdef ___WYLAND_SWITCH_INSTRUCTIONS___
  void _switch_instruction(uint8_t op) {
    switch (op) {
      case set_wtarg64::nop: nop(); break;
      case set_wtarg64::lea: ilea(); break;
      case set_wtarg64::load: iload(); break;
      case set_wtarg64::store: istore(); break;
      case set_wtarg64::mov: imov(); break;
      case set_wtarg64::add: iadd(); break;
      case set_wtarg64::sub: isub(); break;
      case set_wtarg64::mul: imul(); break;
      case set_wtarg64::odiv: idiv(); break;
      case set_wtarg64::mod: imod(); break;
      case set_wtarg64::jmp: ijmp(); break;
      case set_wtarg64::je: ije(); break;
      case set_wtarg64::jne: ijne(); break;
      case set_wtarg64::jl: ijl(); break;
      case set_wtarg64::jg: ijg(); break;
      case set_wtarg64::jle: ijle(); break;
      case set_wtarg64::jge: ijge(); break;
      case set_wtarg64::cmp: icmp(); break;
      case set_wtarg64::xint: ixint(); break;
      case set_wtarg64::loadat: iloadat(); break;
      case set_wtarg64::ret: iret(); break;
      case set_wtarg64::movad: imovad(); break;
      case set_wtarg64::sal: isal(); break;
      case set_wtarg64::sar: isar(); break;
      case set_wtarg64::owthrow: iwthrow(); break;
      case set_wtarg64::clfn: iclfn(); break;
      case set_wtarg64::empl: iemplace(); break;
      case set_wtarg64::push_mmio: ipushmmio(); break;
      case set_wtarg64::pop_mmio: ipopmmio(); break;
      case set_wtarg64::connect_mmio: iconnectmmio(); break;
      case set_wtarg64::deconnect_mmio: ideconnectmmio(); break;
      case set_wtarg64::oand: iand(); break;
      case set_wtarg64::oor: ior(); break;
      case set_wtarg64::oxor: ixor(); break;
      case set_wtarg64::inc: iinc(); break;
      case set_wtarg64::dec: idec(); break;
      case 0xFF: halted = true; break;
      case 0xFE: break;
      default:
        wthrow(std::runtime_error(memberofstr + "():""Invalid instruction opcode: " + std::to_string(op)));
        break;
    }
  }
#endif 

/*
  #pragma region bad...
  void run_step() override {
    if (!halted) {

      if (ip < beg || ip >= end) 
      throw std::out_of_range(
        "Reading out of the local segment.\n"
        "\tflag 'beg':\t" + std::to_string(beg) + "\n"
        "\tflag 'end':\t" + std::to_string(end) + "\n"
        "\tIP (global):\t" + std::to_string(ip) + "\n"
        "\tthread:\t\t" + std::to_string(thread_id)  + "\n"
        "\tlocal IP:\t" + std::to_string(local_ip) + "\n"
        "\tglob.relat:\t" + std::to_string(code_start) + "(use it as base address for disk-file)\n"
        "\tfrom wtarg64::run()"
      );
      
      auto fetched = read();
      local_ip++;

      if (fetched == 0xFF) { halted = true; return; }
      if (fetched == 0xFE) { return; } / * Specific mark for labels. Used for debugging. * /

      wyland_uint key = get_key();
      if (key)  {
        regs.set(REG_KEY, key);
      }
      
      if (fetched >= sizeof(set) / sizeof(set[0])) {
        std::ostringstream oss;
        oss << "Invalid instruction: " << std::hex << std::setw(2) << std::setfill('0') << (int)fetched << "\n"
        "\tfetched:\t[" << (int)fetched<< "]\n"
        "\t4 bytes:\t[" <<  
        format({memory[ip-2],memory[ip-1],memory[ip],memory[ip+1]}, ',') 
        << "]\n"
        "\tCore IP:\t" << std::setw(16) << std::setfill('0') << std::hex << ip  << "\n"
        "\tLocal IP:\t" << std::setw(16) << std::setfill('0') << std::hex << local_ip << "\n"
        "\tbase.add:\t0x" << std::setw(16) << std::setfill('0') << std::hex << base_address << "\n"
        "\trel.add:\t0x"  << std::setw(16) << std::setfill('0') << std::hex << code_start << "\n"
        "\tThread:\t" << std::dec << thread_id;
        throw std::runtime_error(oss.str());
      }

      try {
        #ifdef __WYLAND_DEBUG_PRINT_IP__
        std::cout << "0x" << std::setfill('0') << std::setw(16) << std::hex << ip << ":0x" << std::setw(2) << (int)fetched << std::endl;
        #endif
        (this->*set[fetched])();
      } catch (const std::exception &e) {
        throw runtime::wyland_runtime_error(e.what(), "Instruction Invokation Exception", memberofcstr, typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      } catch (const runtime::wyland_runtime_error &e) {
        throw runtime::wyland_runtime_error(e.what(), e.name(), e.caller(), typeid(e).name(), ip, thread_id, NULL, NULL, end-beg);
      } 
    }

    if (halted) {
      GraphicsModule->shutdown();
      MMIOModule1->shutdown();
      MMIOModule2->shutdown();
    }
  }  

  void run_debug(int max_step = -1) override {
    int i = 0;
    while (!halted && i < max_step) {
      std::cout << "[d]: 0x" << std::setw(16) << std::setfill('0') << std::hex << ip << ": " << (int)memory[base_address + local_ip] << std::endl;
      run_step();
      if (max_step != -1) i++;
    }
  }
  #pragma endregion
  */
};

WYLAND_END