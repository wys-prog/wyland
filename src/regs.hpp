#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

#include "wyland-runtime/wylrt.hpp"

namespace wylma {
  namespace wyland {
    class reg_t {
      private:
      uint8_t  r8 [16]{0}; 
      uint16_t r16[16]{0}; 
      uint32_t r32[16]{0}; 
      uint64_t r64[32]{0};

      public:
      void set(uint8_t to, uint64_t u) {
        if (to < 16) 
          r8[to] = static_cast<uint8_t>(u & 0xFF);  
        else if (to < 32) 
          r16[to - 16] = static_cast<uint16_t>(u & 0xFFFF);  
        else if (to < 48) 
          r32[to - 32] = static_cast<uint32_t>(u & 0xFFFFFFFF); 
        else if (to < 80) 
          r64[to - 48] = u;
        else 
          throw runtime::wyland_out_of_range(std::string("Unexpected register index: " + std::to_string(to)).c_str(), "out of range", "reg_t::set(...)", 0, 0, nullptr, nullptr, 0);
      }

      uint64_t get(uint8_t who) const {
        if (who < 16) 
          return static_cast<uint64_t>(r8[who]);  
        else if (who < 32) 
          return static_cast<uint64_t>(r16[who - 16]);  
        else if (who < 48) 
          return static_cast<uint64_t>(r32[who - 32]);  
        else if (who < 80) 
          return r64[who - 48];  
        else 
        throw runtime::wyland_out_of_range(std::string("Unexpected register index: " + std::to_string(who)).c_str(), "out of range", "reg_t::set(...)", 0, 0, nullptr, nullptr, 0);
      }
    };
  }
}
