#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <functional>
#include <unordered_map>

#include "kokuyo.hpp"
#include "std.hpp"


void handle_arg(std::istringstream &iss, kokuyo::kokuyoVM &vm) {
  std::string arg;
  std::vector<std::string> inputs;
  std::vector<std::string> libs2load;
  std::unordered_map<std::string, std::function<void(kokuyo::array<uint64_t, 32>&, kokuyo::array<uint64_t, 4096>&, std::vector<uint8_t> &)>> table;

  while (iss >> arg) {
    
  }
}

int main(int argc, char const *argv[]) {
  
  

  return 0;
}
