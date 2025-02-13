#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "kokuyo.hpp"
#include "dlapi.h"

namespace stdfs = std::filesystem;

namespace wyland {

  enum class wobject_type {
    res,          // Files in .res
    logfile,      // Files in .log
    cachedir,     // Files in .cache
    vm,           // Files in .vm
    binfile,      // Files in .bin
    functionlib,  // Files in .fnlib
    systemfile,   // Files in .wyls
  };

  class wobject {
  private:
    std::fstream stream;

  public:
    std::string  path;
    wobject_type type;
    std::unordered_map<std::string, std::string> properties;

    void writeself() {
      if (!is_open()) open();
      writeline("type:" + std::to_string(int(type)));
      for (const auto &propertie : properties) 
        writeline(propertie.first + ":" + propertie.second);
    }

    void loadself() {
      while (!eof()) {
        std::string line = readline();
        size_t endname = line.find(':');
        if (endname == std::string::npos) {}
        else {
          auto pname = line.substr(0, endname);
          auto value = line.substr(endname + 1);
          properties[pname] = value;
        }
      }
    }

    bool is_open() { return stream.is_open(); }

    bool open() {
      stream.open(path);
      return stream.is_open();
    }

    std::string readline() {
      std::string line;
      std::getline(stream, line);
      return line;
    }

    void writeline(const std::string &line) {
      stream << line << std::endl;
    }

    const uint8_t *read(std::streamsize count) {
      uint8_t *buff = new uint8_t[count];
      stream.read((char*)(buff), count);
      return buff;
    }

    void write(const uint8_t *data, std::streamsize len) {
      stream.write((const char*)data, len);
    }

    bool eof() { return stream.eof(); }

    std::fstream &get_stream() { return stream; }

    ~wobject() {
      if (stream.is_open()) stream.close();
    }
  };

  class VMHandle {
  private:
    kokuyoVM vm;
    wobject obj;
  public:
    VMHandle() = default;
    
    VMHandle(const std::string &name, stdfs::path path, bool logs)  {
      
    }
  };

  class Wyland {
  private:
    stdfs::path workspace;
    std::unordered_map<std::string, wobject> objects;
    std::unordered_map<std::string, VMHandle> handles;

    std::vector<std::string> to_create = {
      ".wyland/", ".wyland/res/", ".wyland/libs/", 
      ".wyland/vm/", ".wyland/logs/", ".wyland/cache/", 
      ".wyland/shell/", ".wyland/shell/bin", ".wyland/shell/templates/", 
      ".wyland/logs/wyland.log",
    };

    void init() {
      if (!stdfs::exists(".wyland")) {
        std::cout << "Initializing..." << std::endl;
        for (int i = 0; i < to_create.size(); i++) {
          stdfs::path p = stdfs::absolute(to_create[i]);
          stdfs::create_directories(p);
          std::cout << i << ": Created " << p << std::endl; 
        }
      } else {
        std::cout << "Wyland already initialized." << std::endl;
      }
    }

    void create() {}

  public:
    
    Wyland(int argc, char *const argv[]) {

    }

    int WylandExit = 0;
  };
} // wyland

int main(int argc, char *const argv[]) {
  std::cout << "Wyland —— 1.1" << std::endl;
  wyland::Wyland land(argc, argv);

  return land.WylandExit;
}