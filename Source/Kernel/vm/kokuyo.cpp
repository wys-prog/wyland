// コクウキョウ - Kokūkyō
//            - Wys
//            - Wyland
// This version of Kokuyo is modified.
// It built specialy for the Wyland project.
// This code is free, and open source.

#include <cstdint>
#include <functional>
#include <sstream>
#include <stdfloat>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#define BYTE  1
#define WORD  2
#define DWORD 4
#define QWORD 8

template <typename T>
T cast_array(uint8_t *buff, uint64_t len) {
  T tmp = T(); 
  int bwi = (sizeof(T) - 1) * 8; 
  
  for (uint64_t i = 0; i < len && i < sizeof(T); ++i) {
    tmp |= (static_cast<T>(buff[i]) << bwi); 
    bwi -= 8;
  }

  return tmp;
}

class bad_register    : public std::exception {};
class bad_instruction : public std::exception {};
class stack_overflow  : public std::exception {};


class kokuyo {
private:
  uint64_t regs[32]; // 32 registers 
  size_t   ip;
  uint64_t stack[4096];

  std::vector<uint8_t> program;

  uint8_t read8() { return program[ip++]; }

  uint64_t read64() {
    uint8_t buff[8] = {read8(), read8(), read8(), read8(), read8(), read8(), read8(), read8(), };
    return cast_array<uint64_t>(buff, 8);
  }

  std::unordered_map<uint8_t, std::function<void()>> ftable = {
    {0x00, [this]() {}}, // NOP
    {0x01, [this]() {
        uint8_t r = read8();
        uint64_t data = read64();

        if (r < 32) regs[r] = data;
        else throw bad_register();
      }
    }, 
    {0x02, [this]() {
        regs[read8()] = regs[read8()];
      }
    }, 
    {0x03, [this]() {
        regs[read8()] += regs[read8()];
      }
    },
    {0x04, [this]() {
        regs[read8()] -= regs[read8()];
      }
    }, 
    {0x05, [this]() {
        regs[read8()] *= regs[read8()];
      }
    },
    {0x06, [this]() {
        regs[read8()] /= regs[read8()];
      }
    },
    {0x07, [this]() {
        uint8_t a = read8();
        uint8_t b = read8();
        regs[a] = regs[a] % regs[b];
      }
    },
    {0x08, [this]() {}
      
    }
  };

public:
};

