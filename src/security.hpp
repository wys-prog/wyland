#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include "wmmbase.hpp"
#include "wmmio.hpp"

#include "cache.hpp"

WYLAND_BEGIN

/* Naming convention with `security` elements: FunctionName() { body...; } */

#ifndef ___WYLAND_NOT_MAIN_BUILD___

/*namespace cache {
  extern std::vector<WylandMMIOModule*> SecurityMMIOPointers;
}*/

#endif // ___WYLAND_NOT_MAIN_BUILD___

namespace security {
  
  inline void SecurityAddModules(const std::vector<WylandMMIOModule*> &modules) {
    cache::SecurityMMIOPointers.insert(cache::SecurityMMIOPointers.end(), modules.begin(), modules.end());
  }
  
  inline void SecurityShutDownModules() {
    for (const auto&Module:cache::SecurityMMIOPointers) {
      Module->shutdown();
    }
  }
  
}

WYLAND_END