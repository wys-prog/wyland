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

/* ================================= =================== ================================= */
/* ================================= Warch128's includes ================================= */
/* ================================= =================== ================================= */


WYLAND_BEGIN

ARCH_BACK(warch128_backend) 
ARCH_BACK_V(V1)

class warch128 {
private:

public:
};

ARCH_END
ARCH_END

WYLAND_END