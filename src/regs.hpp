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
        else {
          std::string buff = "Unexpected register index: " + std::to_string(to);
          throw runtime::wyland_out_of_range(buff.c_str(), "out of range", "reg_t::set(...)", 0, 0, nullptr, nullptr, 0);
        }
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
        else {
          std::string buff = "Unexpected register index: " + std::to_string(who);
          throw runtime::wyland_out_of_range(buff.c_str(), "out of range", "reg_t::set(...)", 0, 0, nullptr, nullptr, 0);
        }
      }

      wyland_registers wrap() {
        wyland_registers reg;
        
        reg.r8  = reinterpret_cast<uint8_t(*)[16]>(&this->r8);
        reg.r16 = reinterpret_cast<uint16_t(*)[16]>(&this->r16);
        reg.r32 = reinterpret_cast<uint32_t(*)[16]>(&this->r32);
        reg.r64 = reinterpret_cast<uint64_t(*)[32]>(&this->r64);
    
        return reg;
      }
    };
  }
}
