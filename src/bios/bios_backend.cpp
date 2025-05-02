#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wylrt.hpp"

#include "bios_backend.hpp"
#include "bios.hpp"

typedef void(*syscall_callable)(wuint, wyland_registers*);


WYLAND_BEGIN

namespace bios {
  class BiosException : public runtime::wyland_runtime_error {
  private:
  public:
    BiosException(const std::string &what, const std::string &from)
      : runtime::wyland_runtime_error(what.c_str(), "BIOS Exception", from.c_str(), typeid(this).name(), 0, 0, NULL, NULL, -1) {}
  };

  void bwrite(wuint, wyland_registers *regs) {
    putc(*regs->r32[0], stdout);
    fflush(stdout);
  }
  
  syscall_callable syscall_table[] = {
    &bwrite,
  };

  const constexpr static auto max_syscalls = sizeof(wylma::wyland::bios::syscall_table) / sizeof(wylma::wyland::bios::syscall_table[0]);
}

WYLAND_END

extern "C" {
  void bios_backend_interrupt(wuint id, wyland_registers *regs) {
    if (id >= wylma::wyland::bios::max_syscalls) {
      std::stringstream what, name; // Because stringstream are more simple than strings to use...
      what << "invalid interruption: 0x" << std::setw(4) << std::setfill('0') << std::hex << id;
      name << "BIOS/" << __func__;
      throw wylma::wyland::bios::BiosException(what.str(), name.str());
    } else {
      (*wylma::wyland::bios::syscall_table)(id, regs);
    }
  }
  
  void bios_backend_init(const std::vector<wylma::wyland::WylandMMIOModule*> &modules) {
    std::cout << "[b]: BIOS version: " << wfloat_to_str(bios_backend_version()) << std::endl;
    
    for (const auto&module:modules) {
      std::cout << "[b]: initializing module " << module->name() << ":\t" << std::flush;
      if (!module->init()) {
        std::cout << "[FAILLURE]" << std::endl;
        throw wylma::wyland::MMIOModuleException("Unable to initialize MMIO Module: " + module->name(), "BIOS/Backend/" + std::string(__func__));
      } else std::cout << "[SUCCESS]" << std::endl;
    }
  }
  
  wfloat bios_backend_version() { 
    return float_to_wfloat(1.0002); 
  } 
}