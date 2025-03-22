#pragma once

#include <cstdint>
#include <stdexcept>

namespace wylma {
  namespace wyland {
    class reg_t {
      private:
      uint8_t  r8 [16]{0};
      uint16_t r16[16]{0};
      uint32_t r32[16]{0};
      uint64_t r64[16]{0};
      
      public:
      void set(uint8_t to, uint64_t u) {
        if (to < 16) r8[to] = u;
        else if (to < 32) r16[to - 16] = u;
        else if (to < 48) r32[to - 32] = u;
        else if (to < 64) r64[to - 48] = u;
        else throw std::out_of_range("Unexpected register: " + std::to_string(to));
      }
      
      uint64_t get(uint8_t who) {
        if (who < 16) return r8[who];
        else if (who < 32) return r16[who - 16];
        else if (who < 48) return r32[who - 32];
        else if (who < 64) return r64[who - 48];
        else throw std::out_of_range("Unexpected register: " + std::to_string(who));
      }
    };
    
  }
}