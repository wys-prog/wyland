#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include <cstdint>
#include <stdexcept>
#include <string>
#include <cstring>

#include "wyland-runtime/wylrt.hpp"
#include "wc++std.hpp"

namespace wylma {
  namespace wyland {
    class reg_t {
    private:
      alignas(64) uint8_t  r8 [16]{0}; 
      alignas(64) uint16_t r16[16]{0}; 
      alignas(64) uint32_t r32[16]{0}; 
      alignas(64) uint64_t r64[32]{0};
      alignas(64) __uint128_t r128[32]{0}; // Added 128-bit registers

      static constexpr uint8_t len_table[96] = { // Updated table size to include 128-bit registers
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 // 128-bit registers
      };

    public:
      inline constexpr void set(uint8_t to, uint64_t u) {
        switch (len_table[to]) {
          case 1: r8[to] = static_cast<uint8_t>(u); break;
          case 2: r16[to - 16] = static_cast<uint16_t>(u); break;
          case 4: r32[to - 32] = static_cast<uint32_t>(u); break;
          case 8: r64[to - 48] = u; break;
          case 16: r128[to - 80] = static_cast<__uint128_t>(u); break; // Handle 128-bit registers
          default:
          std::string buff = "Unexpected register index: " + std::to_string(to);
          wthrow (runtime::wyland_out_of_range(buff.c_str(), "out of range", "reg_t::set(...)", 0, 0, nullptr, nullptr, 0));
        }
      }

      inline constexpr uint64_t get(uint8_t who) const {
        switch (len_table[who]) {
          case 1: return r8[who];
          case 2: return r16[who - 16];
          case 4: return r32[who - 32];
          case 8: return r64[who - 48];
          case 16: return static_cast<uint64_t>(r128[who - 80]); // Handle 128-bit registers (lower 64 bits)
          default:
          std::string buff = "Unexpected register index: " + std::to_string(who);
          wthrow (runtime::wyland_out_of_range(buff.c_str(), "out of range", "reg_t::get(...)", 0, 0, nullptr, nullptr, 0));
        }
      }

      wyland_registers wrap() {
        wyland_registers reg;
        reg.r8  = reinterpret_cast<uint8_t(*)[16]>(&this->r8);
        reg.r16 = reinterpret_cast<uint16_t(*)[16]>(&this->r16);
        reg.r32 = reinterpret_cast<uint32_t(*)[16]>(&this->r32);
        reg.r64 = reinterpret_cast<uint64_t(*)[32]>(&this->r64);
        reg.r128 = reinterpret_cast<__uint128_t(*)[32]>(&this->r128); // Wrap 128-bit registers
        return reg;
      } 

      static constexpr uint8_t get_len(uint8_t of) {
        if (of >= 96) { // Updated range check
          std::string buff = "Unexpected register index: " + std::to_string(of);
          wthrow (runtime::wyland_out_of_range(buff.c_str(), "out of range", "reg_t::get_len(...)", 0, 0, nullptr, nullptr, 0));
        }
        return len_table[of];
      }
    };
  }
}
