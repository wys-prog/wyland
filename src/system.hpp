#pragma once

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif 

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

std::string get_execution_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
            std::cerr << "[e]: Unable to retrieve executable path." << std::endl;
            wyland_exit(-1);
    }

    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("\\/");
    return (pos != std::string::npos) ? fullPath.substr(0, pos) : fullPath;
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        std::cerr << "[e]: Unable to retrieve executable path (macOS)." << std::endl;
        wyland_exit(-1);
    }

    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("/");
    return (pos != std::string::npos) ? fullPath.substr(0, pos) : fullPath;
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
            std::cerr << "[e]: Unable to retrieve executable path." << std::endl;
            wyland_exit(-1);
    }
    path[count] = '\0';

    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("/");
    return (pos != std::string::npos) ? fullPath.substr(0, pos) : fullPath;
#endif
    }

    std::string file_at_execution_path(const std::string &whichone) {
        auto path = get_execution_path();
    #ifdef _WIN32
        if (!path.ends_with('\\') && !path.ends_with('/')) path += "\\";
    #else
        if (!path.ends_with('/')) path += "/";
    #endif
        path += whichone;
        return path;
    }
    
};

OS os; // Idk why I'm doing that, but for now I'll keep that.
  
WYLAND_END


