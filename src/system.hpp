#pragma once

#include <string>
#include <sstream>

#include "wmmbase.hpp"

WYLAND_BEGIN

class OS {
private: /* We'll add cool stuff here later */
public:
std::string name() const {
#ifdef _WIN64
    return "Windows x64";
#elif defined(_WIN32)
    return "Windows x86";
#elif defined(__APPLE__)
    return "MacOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__unix__)
    return "Unix";
#else 
    return "CoolOS";
#endif // Windows ?
  }

  std::string arch() const {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86-64";
#elif defined(__i386) || defined(_M_IX86)
    return "x86-32";
#elif defined(__aarch64__)
    return "arm-64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#else
    return "nice CPU";
#endif // x64 ?
  }

  std::string  endianness() const {
    unsigned int x = 1;
    return (*(char*)&x == 1) ? "Little Endian" : "Big Endian";
  }

  std::string get_fmt_specs() const {
    std::stringstream ss;
    ss << "OS Name:\t" << this->name() << "\n"
          "Architecture:\t" << this->arch() << "\n"
          "Endianness:\t" << this->endianness() << "\n"
          "===== TYPES =====\n"
          "sizeof int:\t" << sizeof(int) << "\n"
          "sizeof long:\t" << sizeof(long) << "\n"
          "sizeof char:\t" << sizeof(char) << "\n"
          "sizeof ptr:\t" << sizeof(void*) << "\n";
    
    return ss.str();
  }

};

OS os; // Idk why I'm doing that, but for now I'll keep that.
  
WYLAND_END