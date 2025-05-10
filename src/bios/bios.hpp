#pragma once

#include <cstdio>
#include <ctime>
#include <iostream>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wylrt.hpp"

#include "bios_backend.hpp"

/*
#warning ==========================================================================================
#warning                                          TODO
#warning - BIOS/GPU
#warning - BIOS/Threading
#warning - Core/Threading
#warning - BIOS/StdIO
#warning 
#warning ========================================================================================== 
*/

WYLAND_BEGIN

class BIOS {
private:
public:

  void interrupt(wint id, wyland_registers *regs) {
    bios_backend_interrupt(id, regs);
  }

  void init(const std::vector<WylandMMIOModule*> &modules, 
            IWylandGraphicsModule *gm, uint8_t *mmptr, WylandMMIOModule *dsptr, 
            const std::vector<USBDrive*> &usbdrives) {
    bios_backend_init(modules, gm, mmptr, dsptr, usbdrives);
  }

  inline long double version() {
    return bios_backend_version();
  }
};

WYLAND_END