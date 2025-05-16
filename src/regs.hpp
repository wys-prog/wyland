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

#include "sock2.h"

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
            /* This is one of the rare parts I'll explain in detail.
             Essentially, we cannot assume the host's byte order. However, our VM ALWAYS uses big-endian.
             The issue arises when casting, for example, a uint64_t to a uint16_t. The value may differ between platforms.
             To address this, whenever we perform a cast, we invoke the 'correct_byte_order_x' macro.
             Note: 'correct_byte_order_8(x)' does nothing on any platform, but I included it just for fun! :) */
          case 1: r8[to] = correct_byte_order_8(u); break;
          case 2: r16[to - 16] = correct_byte_order_16(u); break;
          case 4: r32[to - 32] = correct_byte_order_32(u); break;
          case 8: r64[to - 48] = u; break;
          case 16: r128[to - 80] = correct_byte_order_128(u); break; // Handle 128-bit registers
          default:
          std::string buff = "Unexpected register index: " + std::to_string(to);
          wthrow (runtime::wyland_out_of_range(buff.c_str(), "out of range", memberofcstr, 0, 0, nullptr, nullptr, 0));
        }
      }

      inline constexpr void set(uint8_t to, __uint128_t u, char) {
        if (to < 80 || to >= 96) {
          std::string buff = "Unexpected register index: " + std::to_string(to);
          wthrow(runtime::wyland_out_of_range(buff.c_str(), "out of range", memberofcstr, 0, 0, nullptr, nullptr, 0));
        }
        r128[to - 80] = (u); // Handle 128-bit registers
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
          wthrow (runtime::wyland_out_of_range(buff.c_str(), "out of range", memberofcstr, 0, 0, nullptr, nullptr, 0));
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
