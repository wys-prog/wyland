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

#include <boost/container/flat_map.hpp>

#include "wyland-runtime/keys.h"
#include "wyland-runtime/wylrt.h"
#include "wyland-runtime/wylrt.hpp"

#include "regs.hpp"
#include "libcallc.hpp"
#include "targets.h"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wformat.hpp"
#include "wtargb.hpp"

WYLAND_BEGIN

#ifdef _WIN32
#define CLEAR "cls"
#define LS "dir"
#else
#define CLEAR "clear"
#define LS "ls"
#endif // WIN ?


class wtargdebugger : public core_base {
private:
  core_base      *core;
  uint64_t        memory_segment_begin;
  uint64_t        memory_segment_end;
  bool            is_system;
  uint64_t        name;
  linkedfn_array *table;
  uint64_t        base;
  bool            is_init = false;
  bool            ask_exit = false;

  uint64_t get_core_ip() {
    if (core) return core->get_ip();
    return 0;
  }

public:
  void init(uint64_t _memory_segment_begin, 
    uint64_t _memory_segment_end, 
    bool _is_system, 
    uint64_t _name, 
    linkedfn_array *_table, 
    uint64_t _base) override {

    memory_segment_begin = _memory_segment_begin;
    memory_segment_end = _memory_segment_end;
    is_system = _is_system;
    table = _table;
    base = _base;
  }

  void get(const std::vector<std::string> &args) {
    for (size_t i = 0; i < args.size(); i++) {
      if (args.empty()) return;
      if (args[i] == "ip") {
        std::cout << std::hex << "[i]: get(ip): " << get_core_ip() << std::endl;
      } else if (args[i] == "memory") {
        std::cout << std::dec << "[i]: get(memory): " << segments::memory_size << " bytes" << std::endl;;
      } else {
        std::cerr << "[e]: get(): unknown get: " << args[i] << std::endl;
      }
    }
  }

  void execute(const std::string &line) {
    auto tokens = split(format(line, string_format_option::trim_str), ' ');
    if (tokens.empty()) return;
    std::string local = tokens[0];
    auto args = std::vector<std::string>{};

    if (tokens.size() > 1) {
      args = std::vector<std::string>(tokens.begin() + 1, tokens.end());
    }
    
    if (local == "clear" || local == "cls") std::system(CLEAR);
    else if (local == "ls" || local == "dir") std::system(LS);
    else if (local == "init") {
      if (!is_init) {
        if (!core) {
          std::cerr << "[e]: <core*> is not a valid pointer." << std::endl;
        } else {
          core->init(memory_segment_begin, memory_segment_end, is_system, name, table, base);
          std::cout << "[i]: done." << std::endl;
        }
      } else std::cout << "already initialized." << std::endl;
    } else if (local == "run") {
      core->step_break_point();
    } else if (local == "run-step" || local == "step") {
      core->run_step();
    } else if (local.empty()) {
      return ;
    } else if (local == "get") {
      get(args);
    } else if (local == "exit") {
      ask_exit = true;
      return ;
    } else {
      std::cout << "[e]: unknown command: " << local << std::endl;
    }
  }

  void run() override {
    std::cout << "[d]: starting debugging on core 0x" 
    << std::hex << reinterpret_cast<uintptr_t>(core) << std::endl;

    while (!ask_exit) {
      std::cout << "wyland (debug)~ " << std::flush;
      std::string line;
      std::getline(std::cin, line);
      execute(line);
    }
  }
};


WYLAND_END