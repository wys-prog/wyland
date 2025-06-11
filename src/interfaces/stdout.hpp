#pragma once

#include <stdio.h>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "interface.hpp"

WYLAND_BEGIN

namespace BuiltIns {

  class StdOutGPU : IWylandGraphicsModule {
  private:
    wulong last_flag;

    void write(char c) {
      last_flag = putc(c, stdout);
    }

    void flush() {
      last_flag = fflush(stdout);
    }

    void clear() {
#ifdef _WIN32
      last_flag = std::system("cls");
#else
      last_flag = std::system("clear");
#endif
    }

  public:
    uint64_t receive_data() override { return last_flag; }

    void send_data(uint64_t data) override {
      int i = 0;
      auto bin = to_bin_v(data);
      
      for (i; i < 4; i += 2) {
        switch (bin[i]) {
          case 0x00: write(bin[i+1]); break;
          case 0x01: flush(); break;
          case 0x02: clear(); break;
          default: break; // Don't throw anything, for compatibility guys.
        }
      }
    }
  };

}


WYLAND_END