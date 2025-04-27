#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include "wmmbase.hpp"
#include "wmmio.hpp"

WYLAND_BEGIN

/* Naming convention with `security` elements: FunctionName() { body...; } */

namespace cache {
  std::vector<WylandMMIOModule*> SecurityMMIOPointers;
}

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