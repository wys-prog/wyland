// コクウキョウ - Kokūkyō
//            - Wys
//            - Wyland
// This version of Kokuyo is modified. 
// It built specialy for the Wyland project.
// This code is free, and open source.

#include <vector>
#include <sstream>
#include <string>
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <typeindex>
#include <typeinfo>

class core {
private:
  // Programs are dynamic. They can change size.
  std::vector<uint8_t> program;

  // Registers are not in memory. Memory didn't exists.
  std::unordered_map<uint8_t, uint64_t> r64 = {
    {'a', 0x0000000000000000},
    {'b', 0x0000000000000000},
    {'c', 0x0000000000000000},
    {'d', 0x0000000000000000},
    {'@', 0x0000000000000000}, // Istruction Pointer (ip)
    {'%', 0x0000000000000000}, // Stack Pointer
  };

  std::unordered_map<uint8_t, uint32_t> r32 = {
    {'a', 0x00000000},
    {'b', 0x00000000},
    {'c', 0x00000000},
    {'d', 0x00000000},
  };

  std::unordered_map<uint8_t, uint16_t> r16 = {
    {'a', 0x0000},
    {'b', 0x0000},
    {'c', 0x0000},
    {'d', 0x0000},
  };

  std::unordered_map<uint8_t, uint8_t> r8 = {
    {'a', 0x00},
    {'b', 0x00},
    {'c', 0x00},
    {'d', 0x00},
  };

  std::unordered_map<uint8_t, std::type_index> opT = {
    {0x00, typeid(uint8_t)}, 
    {0x01, typeid(uint16_t)}, 
    {0x02, typeid(uint32_t)}, 
    {0x03, typeid(uint64_t)},
  };

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

  template <typename T>
  T read() {
    uint8_t *buff = new uint8_t[sizeof(T)];
    for (int i = 0; i < sizeof(T); i++) {
      buff[i] = program[r64['@']++];
    }

    return cast_array<T>(buff, sizeof(T));
  }

public:

};