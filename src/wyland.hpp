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

#include <boost/stacktrace.hpp>

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
#include "wyland.h"

WYLAND_BEGIN

core_base *create_core_ptr(__wtarget target) {
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

bool load_file(std::fstream &file, const wheader_t &header) {
  file.seekg(header.code);
  if (!file.good()) {
    std::cerr << "[e]: failed to seek to code position in disk file." << std::endl;
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

std::vector<libcallc::DynamicLibrary::FunctionType> load_libs(std::fstream &file, const wheader_t &header) {
  std::vector<libcallc::DynamicLibrary::FunctionType> libraries{};

  file.seekg(header.lib);
  if (!file.good()) {
    std::cerr << "[e]: failed to seek to `lib` position in disk file." << std::endl;
    std::cerr << "[w]: process will continue, without libraries. Program can crash without them." << std::endl;
    return libraries;
  }

  std::cout << "[i]: loading libraries..." << std::endl;

  while (!file.eof()) {
    char buff[1]{0};
    file.read(buff, sizeof(buff));
    std::string fullname = "" + buff[0];

    while (buff[0] && !(file.eof())) {
      file.read(buff, sizeof(buff));
      fullname += buff[0];
    }
    
    size_t libend = fullname.find(':');
    if (libend == std::string::npos) {
      std::cerr << "[e]: unable to resolve label `" << fullname << "`, bad format." << std::endl;
      continue;
    }

    std::string libname = fullname.substr(0, libend) + LIB_EXT;
    std::string funcsname = fullname.substr(libend + 1);
    
    if (!std::filesystem::exists(libname)) {
      std::cerr << "[e]: library `" << libname << "`: no such file." << std::endl;
      continue;
    }

    libname = std::filesystem::absolute(libname);
    std::cout << "[i]: " << "loading `" << funname << "` from `" << libname << "`" << std::endl;
    libcallc::DynamicLibrary lib(libname);
    auto funcs = split(funcsname, ",", true);
    for (const auto&func:funcs) {
      try {
        libraries.push_back(lib.loadFunction(func.c_str()));
        std::cout << "[i] loaded function `" << func << "` from `" << libname << "`" << std::endl;
      } catch (const std::runtime_error &e) {
        std::cerr << "[e]: " << e.what() << std::endl;
        continue;
      }
    }
  }

  return libraries;
}

void run_core(core_base *base) {
  if (base == nullptr) {
    std::cerr << "[e]: running with <*base> as invalid pointer." << std::endl;
    exit(-400);
  }

  std::cout << "[i]: running core at 0x" << std::hex << reinterpret_cast<uintptr_t>(base) << std::endl;
  
  try {
    base->run();
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
  } catch (...) {
    std::cerr << "[e]: unknown exception caught at address 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(base) 
              << "\n\tstacktrace:\n" << boost::stacktrace::stacktrace()
              << std::endl;
  }
}

WYLAND_END