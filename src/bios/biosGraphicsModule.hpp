#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "../wyland-runtime/wylrt.hpp"
#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../interfaces/interface.hpp"

WYLAND_BEGIN

namespace bios {
  class BiosStream {
  private:
    std::ostream &stream;
  
  public:

    template <typename T>
    void WriteBytes(const T &__T) {
      // Serialize T  
      auto bin = to_bin_v<T>(__T);
      stream.write((char*)bin.data(), bin.size());
    }

    void Flush() {
      stream.flush();
    }

    
  }; 

  class BiosGraphicsModule : public IWylandGraphicsModule {
  private:
    
  };
}

WYLAND_END