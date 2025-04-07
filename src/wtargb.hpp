#pragma once

#include <cstdint>

#include <boost/container/flat_map.hpp>

namespace wylma {
  namespace wyland {

    class core_base {
    private:
    public:
      virtual void init(uint64_t, uint64_t, bool, uint64_t, linkedfn_array*, uint64_t) { }
      virtual void run() { }
      virtual uint64_t get_ip() { return 0; }
      virtual void run_step() { }
      virtual void step_break_point() { } /* Will read until he don't finds '0xFE' */
      
      virtual ~core_base() { /* Mouehehehehehehhehhehe destroy... This. */ }
    };
  }
  }