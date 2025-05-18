#pragma once

#include <cstdint>

#include <boost/container/flat_map.hpp>

#include "interfaces/interface.hpp"
#include "wmmio.hpp"
#include "bios/bios.hpp"

namespace wylma {
  namespace wyland {

    class wyland_base_core {
    private:
    public:
      virtual void init(uint64_t, uint64_t, bool, uint64_t, linkedfn_array*, uint64_t, 
                        IWylandGraphicsModule*, WylandMMIOModule*, 
                        WylandMMIOModule*, WylandMMIOModule*, BIOS*) { }
      virtual void run() { }
      virtual uint64_t get_ip() { return 0; }
      virtual void run_step() { }
      virtual void step_break_point() { } /* Will read until he don't finds '0xFE' */
      virtual void run_debug(int) { }
      virtual ~wyland_base_core() { /* Mouehehehehehehhehhehe destroy... This. */ }
    };
  }
  }