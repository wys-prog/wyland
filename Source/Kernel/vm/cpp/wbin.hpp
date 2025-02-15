#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace wyland {
  class wbin {
  private:
  public:
    static void read(const std::string &path, std::vector<uint8_t> &data) {
      std::ifstream stream(path);
      if (stream) {
        stream.seekg(0, std::ios::end);
        data.resize(stream.tellg());
        stream.seekg(0, std::ios::beg);
        stream.read((char*)data.data(), data.size());
      }
    }
  };
} // namespace wyland
