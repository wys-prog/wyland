#pragma once

#include <filesystem>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>

#include "libcallc.hpp"
#include "wmutiles.hpp"
#include "system.hpp"

#ifndef THATFUNC
#define THATFUNC __PRETTY_FUNCTION__
#endif

#ifndef WYLAND_WYFS_SAVE_NAME
#warning "WYLAND_CONST_WYFS_SAVE_NAME not defined..."
#define WYLAND_WYFS_SAVE_NAME "WylmaFileSystemEncryptionSaveFile"
#endif

#ifndef WYLAND_WYFS_LOAD_NAME
#warning "WYLAND_CONST_WYFS_LOAD_NAME not defined..."
#define WYLAND_WYFS_LOAD_NAME "WylmaFileSystemEncryptionLoadFile"
#endif

namespace wylma {
namespace wyland {

  using namespace std::string_literals;

  class EnvManagerException : public std::exception {
  private:
    std::stringstream MyWhat;

  public:

    EnvManagerException(const std::string &what, const std::string &from) {
      MyWhat << "from:\t" << from << ":\t" << what;
    }

    const char* what() const noexcept override {
      return MyWhat.str().c_str();
    }
  };

  class BuiltInEnvironnementManager {
  private:
    int16_t (*pWYFSSave)(const char*, const char*);
    int16_t (*pWYFSLoad)(const char*, const char*);
    DynamicLibraryHandle handle;
  public:
    void Init() {
      std::string path = os.get_wyfslib();
      handle = DynamicLibraryLoad(path.c_str());
      pWYFSSave = reinterpret_cast<int16_t (*)(const char*, const char*)>(DynamicLibraryFunc(handle, WYLAND_WYFS_SAVE_NAME));

      if (!pWYFSSave) throw EnvManagerException("Unable to load '" WYLAND_WYFS_SAVE_NAME "' function", THATFUNC);
      
      pWYFSLoad = reinterpret_cast<int16_t (*)(const char*, const char*)>(DynamicLibraryFunc(handle, WYLAND_WYFS_LOAD_NAME));
      
      if (!pWYFSLoad) throw EnvManagerException("Unable to load '" WYLAND_WYFS_LOAD_NAME "' function", THATFUNC);
    }

    int16_t Save(const std::string &root, const std::string &out) {
      return pWYFSSave(root.c_str(), out.c_str());
    }

    int16_t Load(const std::string &archive, const std::string &out) {
      return pWYFSLoad(archive.c_str(), out.c_str());
    }
  };

  inline BuiltInEnvironnementManager GlobalEnvironnementManager;
} // wyland
} // wylma

#error...