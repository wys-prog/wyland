#pragma once

#include <cstdint>

namespace wylma {
  namespace wyland {

    
    class core_base {
      private:
      public:
      virtual void init(uint64_t, uint64_t, bool = false, uint64_t = '?') { }
      virtual void run() { }
      
      virtual ~core_base() { /* Mouehehehehehehhehhehe destroy... This. */ }
    };
  }
  }