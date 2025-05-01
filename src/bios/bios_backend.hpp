#pragma once

#include <cstdio>
#include <ctime>
#include <iostream>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wylrt.hpp"

extern "C" {
  void bios_backend_write(wuint _c);
  void bios_backend_interrupt(wint id, wyland_registers *regs);
  void bios_backend_init(const std::vector<wylma::wyland::WylandMMIOModule*> &modules);
  wfloat bios_backend_version();
}
