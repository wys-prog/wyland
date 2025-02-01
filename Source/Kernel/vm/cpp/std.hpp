// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#include <chrono>
#include <thread>
#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <unordered_map>

namespace kokuyo {
  uint64_t to_uint64(uint8_t* data) {
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) {
      result |= (static_cast<uint64_t>(data[i]) << (8 * i));
    }
    return result;
  }

  std::unordered_map<std::string, std::function<uint64_t(array<uint64_t, 32>&, array<uint64_t, 4096>&, std::vector<uint8_t> &)>> stdios = {
    {
      "std:print", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &stack, std::vector<uint8_t> &memory) {

        return 0;
      }
    }
  };
}