#include <cstdio>
#include <ctime>
#include <iostream>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wylrt.hpp"

#include "bios_backend.hpp"
#include "bios.hpp"

extern "C" {

  void bios_backend_write(wuint _c) {
    putc(_c, stdout);
    fflush(stdout);
  }
  
  void bios_backend_interrupt(wint id, wyland_registers *regs) {
    
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
    return float_to_wfloat(1.0001); 
  } 
}