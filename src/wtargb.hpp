#pragma once

#include <cstdint>

#include <boost/container/flat_map.hpp>

namespace wylma {
  namespace wyland {

    typedef boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> wlinkfns;
    class core_base {
      private:
      public:
      virtual void init(uint64_t, uint64_t, bool, uint64_t, linkedfn_array*, uint64_t){ }
      virtual void run() { }
      
      virtual ~core_base() { /* Mouehehehehehehhehhehe destroy... This. */ }
    };
  }
  }