#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "kokuyo.hpp"

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::ifstream file(argv[1], std::ios::binary);
  // Load the first boot sector.
  // In x87, the first 4096 bytes are the boot sector.
  std::vector<uint8_t> boot_sector(4096);
  file.read((char*)boot_sector.data(), 4096);
  file.close();

  wyland::kokuyoVM vm;
  vm.invoke(boot_sector);

  std::cout << "Tracer: " << std::endl;
  for (const auto &trace : vm.get_trace()) {
    std::cout << trace << std::endl;
  }

  return 0;
}