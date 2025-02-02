// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#include <chrono>
#include <thread>
#include <string>
#include <vector>
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

  std::unordered_map<std::string, std::function<void(array<uint64_t, 32>&, array<uint64_t, 4096>&, std::vector<uint8_t> &)>> stdios = {
    {
      "std:print", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory) {
        std::string buff;
        uint64_t i = 0x0000000000000000;
        while (char(memory[regs[0]+i])) buff += memory[regs[0]+i]; 

        std::cout << buff;

        regs[0] = 0x0000000000000000;
      }
    },
    {
      "std:input", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory) {
        char *buff = new char[regs[0]];
        std::cin.read(buff, regs[0]);

        regs[0] = regs[0] - std::cin.gcount();
        for (uint64_t i = 0; i < regs[0]; i++) memory[regs[1]+i] = buff[i];

        regs[0] = 0x0000000000000000;
      }
    },
    {
      "std:write", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory) {
        std::string buff;
        uint64_t i = 0x0000000000000000;
        while (char(memory[regs[0]+i])) buff += memory[regs[0]+i]; 
        std::fstream *file = reinterpret_cast<std::fstream*>(regs[1]);

        if (!file || !file->is_open()) regs[0] =  uint64_t(-1);
        else (*file) << buff;

        if (file->fail()) regs[0] =  uint64_t(-1);
        else regs[0] = 0x0000000000000000;
      }
    }, 
    {
      "std:read", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory) {
        std::fstream *file = reinterpret_cast<std::fstream*>(regs[1]);
        if (!file || !file->is_open()) regs[0] =  uint64_t(-1);
        else {
          size_t max = regs[0];
          char *buff = new char[max];

          file->read(buff, max);
          
          regs[0] = (max-file->gcount());
          
          for (uint64_t i = 0; i < regs[0]; i++) memory[regs[1]+i] = buff[i];
          
        }
      }
    }, 
    {
      "std:open", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory)  {
        std::string filename;
        uint64_t i = 0x0000000000000000;
        while (char(memory[regs[0]+i])) filename += memory[regs[0]+i];
        std::fstream file(filename);
        
        if (!file.is_open()) regs[0] =  uint64_t(-1);
        else {
          void *ptr = &file;
          regs[0] = (uint64_t)ptr;
        }
      }
    }, 
    {
      "std:close", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory)  {
        std::fstream *file = reinterpret_cast<std::fstream*>(regs[1]);
        if (!file || !file->is_open()) regs[0] =  uint64_t(-1);
        else file->close();
      }
    },
  };

  std::unordered_map<std::string, std::function<void(array<uint64_t, 32>&, array<uint64_t, 4096>&, std::vector<uint8_t> &)>> stdtime = {
    {
      "std:time", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory)  {
        regs[0] = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      }
    },
    {
      "std:date", [](array<uint64_t, 32> &regs, array<uint64_t, 4096> &, std::vector<uint8_t> &memory)  {
        
      }
    }
  };
}