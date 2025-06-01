#pragma once

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

#include <filesystem>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "wmmbase.hpp"
#include "wmutiles.hpp"

#ifndef WYLAND_WYFS_LIB_PATH
#warning "WYLAND_WYFS_LIB_PATH not defined.."
#define WYLAND_WYFS_LIB_PATH "./lib/wyfs"
#define WYLAND_WYFS_USE_PREFIX 1
#endif

#define FROM_ME __PRETTY_FUNCTION__

WYLAND_BEGIN

class system_exception : public std::exception {
private:
  std::stringstream MyWhat;

public:
  system_exception(const std::string &what, const std::string &from) {
    MyWhat << "from:\t" << from << ":\t" << what;
  }

  const char* what() const noexcept override {
    return MyWhat.str().c_str();
  }
};

class OS {
private: /* We'll add cool stuff here later */
public:
  static std::string name() {
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
#error "WAIT WAIT WAIT ! YOU CAN'T COMPILE ON THIS OS !"
#endif // _WIN64
  }

  static std::string arch() {
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

  static std::string endianness() {
    unsigned int x = 1;
    return (*(char*)&x == 1) ? "Little Endian" : "Big Endian";
  }

  static std::string get_fmt_specs() {
    std::stringstream ss;
    ss << "OS Name:\t" << name()
       << "\n"
          "Architecture:\t"
       << arch()
       << "\n"
          "Endianness:\t"
       << endianness()
       << "\n"
          "===== TYPES =====\n"
          "sizeof int:\t"
       << sizeof(int)
       << "\n"
          "sizeof long:\t"
       << sizeof(long)
       << "\n"
          "sizeof char:\t"
       << sizeof(char)
       << "\n"
          "sizeof ptr:\t"
       << sizeof(void*) << "\n";

    return ss.str();
  }

  static std::string get_execution_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
      std::cerr << "[e]: Unable to retrieve executable path." << std::endl;
      return "";
    }

    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("\\/");
    return (pos != std::string::npos) ? fullPath.substr(0, pos) : fullPath;
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
      std::cerr << "[e]: Unable to retrieve executable path (macOS)." << std::endl;
      return "";
    }

    std::string fullPath(path);
    auto pos = fullPath.find_last_of('/');
    return (pos != std::string::npos) ? fullPath.substr(0, pos) : fullPath;
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
      std::cerr << "[e]: Unable to retrieve executable path." << std::endl;
      return "";
    }
    path[count] = '\0';

    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("/");
    return (pos != std::string::npos) ? fullPath.substr(0, pos) : fullPath;
#endif
  }

  static std::string file_at_execution_path(const std::string& whichone) {
    auto path = get_execution_path();
#ifdef _WIN32
    if (!path.ends_with('\\') && !path.ends_with('/'))
      path += "\\";
#else
    if (!path.ends_with('/'))
      path += "/";
#endif
    path += whichone;
    return path;
  }

  static std::filesystem::path get_appdata_directory() {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata)
      return fs::path(appdata);
    // Fallback: use local AppData
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData)
      return fs::path(localAppData);
    // Ultimate fallback
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile)
      return std::filesystem::path(userProfile) / "AppData" / "Roaming";
    return std::filesystem::current_path(); // fallback
#elif __APPLE__
    const char* home = std::getenv("HOME");
    if (home)
      return std::filesystem::path(home) / "Library" / "Application Support";
    return std::filesystem::current_path(); // fallback
#else // Linux and others
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg)
      return std::filesystem::path(xdg);
    const char* home = std::getenv("HOME");
    if (home)
      return std::filesystem::path(home) / ".config";
    return std::filesystem::current_path(); // fallback
#endif
  }

  static std::string get_env() {
    if (std::filesystem::exists("./.wyland/")) {
      return std::filesystem::absolute("./.wyland/").string();
    }

    if (std::filesystem::exists(get_appdata_directory() / ".wyland/")) {
      return std::filesystem::absolute(get_appdata_directory() / ".wyland/").string();
    }

    std::cout << "[e]: No global .wyland directory found. Do you want to create one ? [y/n]" << std::endl;
    std::string answer;
    std::cin >> answer;
    if (is_answer_yes(answer)) {
      create_env();
      return get_appdata_directory() / ".wyland/";
    }

    return "";
  }

  static void create_env() {
    static const std::vector<std::filesystem::path> paths {
      "/bin",  "/sbin", "/lib", "/cfg",
      "/etc",  "/tmp",  "/var",  "/dev",
      "/proc", "/mnt",  "/boot", "/sys",
    };

    std::filesystem::create_directory(get_appdata_directory() / ".wyland/");
    const auto prefix = get_appdata_directory() / ".wyland/";
    for (const auto &path:paths) {
      std::filesystem::create_directory(prefix / path);
    }
  }

  static std::string get_wyfslib() {
#if WYLAND_WYFS_USE_PREFIX
    std::filesystem::path prefix = get_env();
#else 
    std::filesystem::path prefix = "";
#endif

    if (!std::filesystem::exists(prefix / WYLAND_WYFS_LIB_PATH)) {
      throw system_exception("cannot find wyfs dynamic library... (I'm very sad now..)", FROM_ME);
    }

    return std::filesystem::absolute(prefix / WYLAND_WYFS_LIB_PATH);
  }
};

inline OS os;

WYLAND_END
