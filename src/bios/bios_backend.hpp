#pragma once

#include <cstdio>
#include <ctime>
#include <iostream>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../interfaces/interface.hpp"
#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wylrt.hpp"

extern "C" {
  void bios_backend_interrupt(wuint id, wyland_registers *regs);
  void bios_backend_init(const std::vector<wylma::wyland::WylandMMIOModule*> &modules, wylma::wyland::IWylandGraphicsModule *gm);
  long double bios_backend_version();
}
