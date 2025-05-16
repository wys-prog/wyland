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

ARCH_BACK(warch128_backend) 
ARCH_BACK_V(V1)

class register_index_error : runtime::wyland_runtime_error {
public:
  register_index_error(const std::string &what, const std::string &from)
    : runtime::wyland_runtime_error(what.c_str(), "register_index_error", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0)
  {}
};

class warch128_regs {
private:
  wui8    bmmx[16];
  wui16   wmmx[16];
  wui32   dmmx[16];
  wui64   qmmx[32];
  wui128  dqmmx[16];

public:
  wui8 get8(wui8 i) {
    if (i < 16) {
      wthrow register_index_error("too big index (max 16 for bmmX registers)", memberofstr);
    }

    return bmmx[i];
  }

  wui16 get16(wui8 i) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for wmmX registers)", memberofstr);
    }

    return wmmx[i];
  }

  wui32 get32(wui8 i) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for dmmX registers)", memberofstr);
    }

    return dmmx[i];
  }

  wui64 get64(wui8 i) {
    if (i >= 32) {
      wthrow register_index_error("too big index (max 32 for qmmX registers)", memberofstr);
    }

    return qmmx[i];
  }

  wui128 get128(wui8 i) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for dqmmX registers)", memberofstr);
    }

    return dqmmx[i];
  }

  void set8(wui8 i, wui8 value) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for bmmX registers)", memberofstr);
    }

    bmmx[i] = value;
  }

  void set16(wui8 i, wui16 value) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for wmmX registers)", memberofstr);
    }

    wmmx[i] = value;
  }

  void set32(wui8 i, wui32 value) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for dmmX registers)", memberofstr);
    }

    dmmx[i] = value;
  }

  void set64(wui8 i, wui64 value) {
    if (i >= 32) {
      wthrow register_index_error("too big index (max 32 for qmmX registers)", memberofstr);
    }

    qmmx[i] = value;
  }

  void set128(wui8 i, wui128 value) {
    if (i >= 16) {
      wthrow register_index_error("too big index (max 16 for dqmmX registers)", memberofstr);
    }

    dqmmx[i] = value;
  }
};

ARCH_END
ARCH_END

WYLAND_END