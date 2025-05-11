#pragma once

#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <new>

#ifdef WYLAND_STACKTRACE
#include "boost/stacktrace.hpp"
#endif // ? WYLAND_STACKTRACE

#include "boost/container/flat_map.hpp"

#include "regs.hpp"
#include "wyland-runtime/wylrt.h"
#include "wyland-runtime/wylrt.hpp"
#include "libcallc.hpp"
#include "targets.h"
#include "wtarget64.hpp"
#include "wtarget32.hpp"
#include "wformat.hpp"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wfiles.h"
#include "wtargb.hpp"
#include "interfaces/exInterface.hpp"
#include "interfaces/ConsoleGraphics.hpp"
#include "wmmio.hpp"
#include "wmutiles.hpp"
#include "disk.hpp"
#include "security.hpp"
#include "bios/bios.hpp"
#include "bios/bios_usb.hpp"
#include "bios/bios_disk.hpp"
#include "bios/stdusb/stdusb.hpp"
#include "wyland_config.hpp"
#include "updates/updater.hpp"
#include "wyland.h"

WYLAND_BEGIN

using namespace std::string_literals;
using namespace std::chrono_literals;

typedef struct {
  std::string path;
  std::vector<std::pair<uint32_t, std::string>> funcs;
} rawlib;

namespace cache {
  boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> linked_funcs{};
  std::vector<libcallc::DynamicLibrary> libraries     = {};
  IWylandGraphicsModule *GraphicsModulePtr            = nullptr;
  WylandMMIOModule      *MMIOModule1Ptr               = nullptr;
  WylandMMIOModule      *MMIOModule2Ptr               = nullptr;
  IWylandDiskModule     *DiskModulePtr                = nullptr;
  BIOS                  *BiosPtr                      = nullptr;
}

void clear_ressources() {
  security::SecurityShutDownModules();
  destroy(cache::SecurityMMIOPointers);
  destroy(cache::linked_funcs);
  destroy(cache::libraries);
  destroy(cache::IExternalGraphicsModuleHandles);
  destroy(cache::WylandMMIOModuleHandles);
  destroy(cache::WylandDiskModuleBuffer);
  cache::ReadBlockBuffer.fill(0x00000000);
  delete memory;
  delete cache::GraphicsModulePtr;
  delete cache::MMIOModule1Ptr;
  delete cache::MMIOModule2Ptr;
  delete cache::DiskModulePtr;
  memory = nullptr;
  cache::GraphicsModulePtr = nullptr;
  cache::MMIOModule1Ptr = nullptr;
  cache::MMIOModule2Ptr = nullptr;
  cache::DiskModulePtr = nullptr;
  cache::BiosPtr = nullptr;
  cache::ReadBlockIndex = 0;
  for (const auto&usb:cache::USBDrivePointersCache) {
    delete usb;
  }
  destroy(cache::USBDevices);
  destroy(cache::USBDrivePointersCache);
}

void wyland_exit(int _code = 0) {
  std::cout << "[i]: exiting with exit code: " << std::dec << _code << std::endl;
  clear_ressources();
  exit(_code);
}

void wyland_terminate(void) {
  std::cerr << "[e]: wyland_terminate called, terminating program. Ressources will be cleared." << std::endl;
  clear_ressources();
  exit(-1);
}

core_base *create_core_ptr(wtarget target) {
  if (target == wtarg64) {
    auto ptr = new corewtarg64();

    std::cout << "[i]: object " << typeid(*ptr).name() << " created at " 
    << "0x" << std::hex << reinterpret_cast<uintptr_t>(ptr) << std::endl;

    return ptr;
  } else if (target == wtarg32) {
    auto ptr = new corewtarg32;

    std::cout << "[i]: object " << typeid(*ptr).name() << " created at " 
    << "0x" << std::hex << reinterpret_cast<uintptr_t>(ptr) << std::endl;

    return ptr;
  }

  return nullptr;
}

bool allocate_memory(size_t size) {
  try {
    memory = new uint8_t[size]();

    std::cout << "[i]: memory segment created at: 0x" << std::hex << std::uintptr_t(&memory) << std::endl;
    std::cout << "[i]: " << std::dec << size << " bytes allocated" << std::hex << std::endl;
    global::memory_size = size;
    return true;
  } catch (const std::bad_alloc &e) {
    std::cerr << "[e]: unable to allocate memory segment: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[e]: error while allocating memory segment: " << e.what() << std::endl;
  } catch (...) { std::cerr << "[e]: unknown exception occured..." << std::endl; }

  return false;
}

bool load_file(std::fstream &file, const wheader_t &header) {
  file.clear();
  file.seekg(header.code);
  if (!file.good()) {
    std::cerr << "[e]: failed to seek to code position in disk file: " << std::hex << header.code << std::endl;
    return false;
  }

  std::cout << "[i]: code region beg: " << header.code << std::endl;
  code_start = header.code;

  size_t i = 0;
  while (!file.eof() && i < SYSTEM_SEGMENT_SIZE && file.tellp() < (std::streampos)header.lib) {
    char buff[1]{0};
    file.read(buff, sizeof(buff));
    memory[SYSTEM_SEGMENT_START+i] = buff[0];
    i++;
  }

  return true;
}

std::vector<std::pair<uint32_t, std::string>> get_funcs_from_lib(const std::string &string) {
  std::vector<std::pair<uint32_t, std::string>> funcs;
  std::istringstream iss(string);
  std::string funcname;
  while (std::getline(iss, funcname, ',')) {
    size_t id_beg = funcname.find(':');

    if (id_beg == std::string::npos) {
      std::cerr << "[e]: invalid libname format. Must have libpath%ID:funcA,ID:funcB,ID:funcC,<...>, with ID: uint32" << std::endl;
      std::cerr << "[e]: with line: " << funcname << std::endl;
      wyland_exit(-120);
    }

    std::string id = funcname.substr(0, id_beg);
    std::string realname = format(funcname.substr(id_beg + 1));

    if (trim(format(id)).empty() && trim(format(realname)).empty()) break;

    try {
      uint32_t uintID = std::stoul(id);
      funcs.push_back({uintID, realname});
    } catch (const std::exception &e) {
      std::cerr << "[e]: C++ Exception during string-conversion (to uint32): " << e.what() << std::endl;
      wyland_exit(-120); 
    }
  }

  return funcs;
}

#pragma region IMPORTANT 
/*
  Starting from std:wy2.6, the format for external functions has been updated.
  The new format is as follows:
  <library_path>%ID:FUNCTION_NAME,ID:FUNCTION_NAME,ID:FUNCTION_NAME,<...>
*/
#pragma endregion

std::vector<rawlib> get_libnames(std::fstream &file, std::streampos max, const wheader_t &header, bool fmt_names = true) {
  std::vector<rawlib> libs = {};

  file.seekg(header.lib);
  if (!file.good()) {
    std::cerr << "[e]: failed to seek to `lib` position in disk file." << std::endl;
    std::cerr << "[w]: process will continue, without libraries. Program can crash without them." << std::endl;
    return {};
  }

  while (file.tellg() < max) {
    char buff[1]{0};
    file.read(buff, sizeof(buff));
    std::string rawstring = "";
    rawstring.push_back(buff[0]);
    // load other chars
    while (buff[0] && !(file.eof()) && (file.tellg() < max)) {
      file.read(buff, sizeof(buff));
      rawstring += buff[0];
    }

    size_t libname_end = rawstring.find('%');
    if (libname_end == std::string::npos) continue;

    std::string libname = rawstring.substr(0, libname_end) + LIB_EXT;
    std::string funcs = rawstring.substr(libname_end + 1);

    if (fmt_names) libname = format(libname);
    if (!std::filesystem::exists(libname)) {
      std::cerr << "[e]: library `" << libname << "`: no such file." << std::endl;
      continue;
    }

    libname = std::filesystem::absolute(libname).string();
    libs.push_back({
      .path = libname, 
      .funcs = get_funcs_from_lib(funcs)
    });

  }
  
  return libs;
}

void load_libs(std::fstream &file, std::streampos max, const wheader_t &header, bool fmt_names = true) {
  std::cout << "[i]: loading libraries" << std::endl;

  auto libs = get_libnames(file, max, header, fmt_names);
  for (const auto& lib : libs) {
    try {
      cache::libraries.push_back(libcallc::DynamicLibrary(lib.path));
      for (const auto& func : lib.funcs) {
        std::cout << "[i]: loading from `" << lib.path << "`: (" << func.first << ") `" << func.second << "`" << std::endl;
        auto efunc = cache::libraries.back().loadFunction(func.second.c_str());
        if (!efunc) {
          std::cerr << "[e]: Failed to load function `" << func.second << "` from `" << lib.path << "`" << std::endl;
          continue;
        }
        cache::linked_funcs.insert({func.first, efunc});
      }
    } catch (const std::exception& e) {
      std::cerr << "[e]: Failed to load library `" << lib.path << "`: " << e.what() << std::endl;
    }
  }

  std::sort(cache::linked_funcs.begin(), cache::linked_funcs.end());

  std::cout << "[i]: " << std::dec << cache::linked_funcs.size() << " functions loaded." << std::endl;
}

void loadGraphicsModule(const std::string &path) {
  if (path == "console" || path == "consgraphs") {
    cache::GraphicsModulePtr = new WylandConsoleGraphicsModule();
    std::cout << "[i]: using built-in WylandConsoleGraphicsModule GPU" << std::endl;
    return;
  } else if (path == "horrible-built-in-gpu") {
    cache::GraphicsModulePtr = new HorribleGraphicsModule();
    std::cerr << "[w]: that's the worst GPU I ever seen..." << std::endl;
    return;
  }

  if (path.empty()) {
    cache::GraphicsModulePtr = new IWylandGraphicsModule();
  } else if (!std::filesystem::exists(path)) {
    std::cerr << "[e]: `" << path << "` no such file. Loading default GraphicsModule" << std::endl;
    cache::GraphicsModulePtr = new IWylandGraphicsModule();
  } else if (path == "SDL" || "sdl") {
    // TODO
  } else {
    cache::GraphicsModulePtr = loadIExternalGraphicsModule(path);
    std::cout << "[i]: new IExternalGraphicsModule loaded at: 0x" << std::hex << reinterpret_cast<uintptr_t>(cache::GraphicsModulePtr) << std::endl; 
  }
}

WylandMMIOModule *loadMMIOModule(const std::string &path) {
  if (path.empty()) {
    return new WylandMMIOModule();
  } else if (!std::filesystem::exists(path)) {
    std::cerr << "[e]: " << path << " no such file. Loading default GraphicsModule" << std::endl;
    return new WylandMMIOModule();
  } else {
    auto ptr = loadIExternalMMIOModule(path);
    std::cout << "[i]: new IExternalMMIOModule loaded at: 0x" << std::hex << reinterpret_cast<uintptr_t>(cache::GraphicsModulePtr) << std::endl; 
    return ptr;
  }

  return new WylandMMIOModule();
}

void loadUSBDevices(const std::vector<std::string> &devices) {
  for (const auto &device:devices) {
    std::string dname = "", dargs = "";
    if (device.contains(':')) {
      dname = device.substr(0, device.find(':'));
      dargs = device.substr(device.find(':'));
    } else {
      dname = device;
    }

    if (dname.starts_with("_w") && stdusb_devices.find(dname) != stdusb_devices.end()) {
      // Built-in USB drive
      cache::USBDevices.push_back(stdusb_devices[dname].Instantiate(dargs));
    } else {
      IExternalUSBDrive Base;
      cache::USBDevices.push_back(Base.Instantiate(dargs));
    }
  }
}

void loadModules(const std::string &pathGraphics, const std::string &m1, const std::string &m2, w_dfstream &stream) {
  loadGraphicsModule(pathGraphics);
  cache::MMIOModule1Ptr = loadMMIOModule(m1);
  cache::MMIOModule2Ptr = loadMMIOModule(m2);
  cache::DiskModulePtr = new IWylandDiskModule(stream);
  cache::BiosPtr = new BIOS();
}

void run_core(core_base *base, bool debug = false, int max = -1) {
  if (base == nullptr) {
    std::cerr << "[e]: running with <base*> as invalid pointer." << std::endl;
    exit(-400);
  }

  std::cout << "[i]: running core at 0x" << std::hex << reinterpret_cast<uintptr_t>(base) << std::endl;
  
  try {
    if (debug) base->run_debug(max);
    else base->run();
    clear_ressources();
    return ;
  } catch (const std::invalid_argument& e) {
    std::cerr << "[e]: invalid argument exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE
              << std::endl;
  } catch (const std::runtime_error& e) {
    std::cerr << "[e]: runtime error exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what() 
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE << std::endl;
  } catch (const std::out_of_range& e) {
    std::cerr << "[e]: out of range exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE << std::endl;
  } catch (const std::logic_error& e) {
    std::cerr << "[e]: logic error exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE << std::endl;
  } catch (const std::bad_alloc& e) {
    std::cerr << "[e]: bad allocation exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE << std::endl;
    std::cerr << "\texecution stopped after a bad allocation." << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "[e]: general exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE << std::endl;
  } catch (const runtime::wyland_runtime_error &e) {
    std::cerr << "[e]: wyland error caught at address 0x"
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.fmterr("\t")
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE
              << std::endl;
  } catch (const wylrterror &e) {
    runtime::wyland_runtime_error error(e);
    std::cerr << "[e]: wyland (C) error caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << error.fmterr("\t")
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE
              << std::endl;
  } catch (...) {
    std::cerr << "[e]: unknown exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\tstacktrace:\n" << WYLAND_GET_STACKTRACE
              << std::endl;
  }

  wyland_exit(-100);
}

WYLAND_END