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

#include "boost/stacktrace.hpp"
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
#include "wyland.h"

WYLAND_BEGIN

namespace cache {
  boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> linked_funcs{};
  std::vector<libcallc::DynamicLibrary> libraries;
  IWylandGraphicsModule *GraphicsModulePtr;
}

void clear_ressources() {
  cache::linked_funcs.clear();
  cache::libraries.clear();
  delete memory;
  delete cache::GraphicsModulePtr;
  memory = nullptr;
  cache::GraphicsModulePtr = nullptr;
}

void wyland_exit(int _code = 0) {
  std::cout << "[i]: exiting with exit code: " << std::dec << _code << std::endl;
  clear_ressources();
  exit(_code);
}

core_base *create_core_ptr(wtarget target) {
  if (target == wtarg64) {
    auto ptr = new corewtarg64;

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
    memory = new uint8_t[size];

    std::cout << "[i]: memory segment created at: 0x" << std::hex << std::uintptr_t(&memory) << std::endl;
    std::cout << "[i]: " << std::dec << size << " bytes allocated" << std::hex << std::endl;
    segments::memory_size = size;
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

  size_t i = 0;
  while (!file.eof() && i < SYSTEM_SEGMENT_SIZE) {
    char buff[1]{0};
    file.read(buff, sizeof(buff));
    memory[SYSTEM_SEGMENT_START+i] = buff[0];
    i++;
  }

  return true;
}

void load_libs(std::fstream &file, const wheader_t &header) {
  file.seekg(header.lib);
  if (!file.good()) {
    std::cerr << "[e]: failed to seek to `lib` position in disk file." << std::endl;
    std::cerr << "[w]: process will continue, without libraries. Program can crash without them." << std::endl;
    return;
  }

  std::cout << "[i]: loading libraries" << std::endl;

  while (!file.eof()) {
    char buff[1]{0};
    file.read(buff, sizeof(buff));
    std::string fullname = "";
    fullname.push_back(buff[0]);

    while (buff[0] && !(file.eof())) {
      file.read(buff, sizeof(buff));
      fullname += buff[0];
    }
    
    size_t libend = fullname.find('/');
    if (libend == std::string::npos) continue;

    std::string libname = fullname.substr(0, libend) + LIB_EXT;
    std::string funcsname = fullname.substr(libend + 1);
    
    if (!std::filesystem::exists(libname)) {
      std::cerr << "[e]: library `" << libname << "`: no such file." << std::endl;
      continue;
    }

    libname = std::filesystem::absolute(libname);
    std::cout << "[i]: " << "loading `" << funcsname << "` from `" << libname << "`" << std::endl;
    cache::libraries.push_back(libcallc::DynamicLibrary());
    cache::libraries[cache::libraries.size()].loadLibrary(libname.c_str());
    auto funcs = split(funcsname, ',');

    for (const auto&func:funcs) {
      try {
        auto parts = split(func, ':');
        if (parts.size() > 2) throw std::invalid_argument("invalid format. Function format must be <x:str>, where x is the ID (an integer) and str the name of the function.");
        auto id = std::stoll(parts[0]);
        cache::linked_funcs.insert({id, cache::libraries[cache::libraries.size()].loadFunction(parts[1].c_str())});
        std::cout << "[i]: loaded function `" << func << "` from `" << libname << "`" << std::endl;
      } catch (const std::runtime_error &e) {
        std::cerr << "[e]: " << e.what() << std::endl;
        continue;
      } catch (const std::invalid_argument &e) {
        std::cerr << "[e]: " << e.what() << std::endl;
        continue;
      } catch (const std::out_of_range &e) {
        std::cerr << "[e]: " << e.what() << std::endl;
        continue;
      } catch (const std::exception &e) {
        std::cerr << "[e]: " << e.what() << std::endl;
        continue;
      }
    }
  }

  std::sort(cache::linked_funcs.begin(), cache::linked_funcs.end());
  
  boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> fn_map(
    cache::linked_funcs.begin(), cache::linked_funcs.end()
  );

  std::cout << "[i]: " << std::dec << fn_map.size() << " functions loaded." << std::endl;
}

void loadGraphicsModule(const std::string &path) {
  std::cout << "[i]: loading GraphicsModule" << std::endl;
  if (path.empty()) {
    cache::GraphicsModulePtr = new IWylandGraphicsModule();
  } else if ( !std::filesystem::exists(path)) {
    std::cerr << "[e]: " << path << " no such file. Loading default GraphicsModule" << std::endl;
    cache::GraphicsModulePtr = new IWylandGraphicsModule();
  } else {
    cache::GraphicsModulePtr = loadIExternalGraphicsModule(path);
    std::cout << "[i]: new IExternalGraphicsModule loaded at: " << std::hex << reinterpret_cast<uintptr_t>(cache::GraphicsModulePtr) << std::endl; 
  }
}

void run_core(core_base *base) {
  if (base == nullptr) {
    std::cerr << "[e]: running with <base*> as invalid pointer." << std::endl;
    exit(-400);
  }

  std::cout << "[i]: running core at 0x" << std::hex << reinterpret_cast<uintptr_t>(base) << std::endl;
  
  try {
    base->run();
    clear_ressources();
    return ;
  } catch (const std::invalid_argument& e) {
    std::cerr << "[e]: invalid argument exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what() 
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace()
              << std::endl;
  } catch (const std::runtime_error& e) {
    std::cerr << "[e]: runtime error exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what() 
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace() << std::endl;
  } catch (const std::out_of_range& e) {
    std::cerr << "[e]: out of range exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace() << std::endl;
  } catch (const std::logic_error& e) {
    std::cerr << "[e]: logic error exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace() << std::endl;
  } catch (const std::bad_alloc& e) {
    std::cerr << "[e]: bad allocation exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace() << std::endl;
    std::cerr << "\texecution stopped after a bad allocation." << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "[e]: general exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.what()
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace() << std::endl;
  } catch (const runtime::wyland_runtime_error &e) {
    std::cerr << "[e]: wyland error caught at address 0x"
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << e.fmterr("\t")
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace()
              << std::endl;
  } catch (const wylrterror &e) {
    runtime::wyland_runtime_error error(e);
    std::cerr << "[e]: wyland (C) error caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\twhat(): " << error.fmterr("\t")
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace()
              << std::endl;
  } catch (...) {
    std::cerr << "[e]: unknown exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace()
              << std::endl;
  }

  wyland_exit(-100);
}

WYLAND_END