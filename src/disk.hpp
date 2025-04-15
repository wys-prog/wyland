#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "wmmbase.hpp"
#include "wmmio.hpp"
#include "wyland-runtime/wylrt.h"

WYLAND_BEGIN

namespace cache {
  std::vector<wuint> WylandDiskModuleBuffer{};
}

class IWylandDiskModule : public WylandMMIOModule {
private:
  std::fstream handle;
  std::string path;
  wuint last_get = 0x00000000;

  void disk_error(const std::string &from, const std::string &what) {
    throw runtime::wyland_runtime_error(what.c_str(), "disk error", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0);
  }

  std::vector<wbyte> compile_cache() {
    std::vector<wbyte> buff{};
    for (const auto &byte : cache::WylandDiskModuleBuffer) {
      auto bytes = to_bin_v(byte);
      buff.insert(buff.end(), bytes.begin(), bytes.end());
    }

    return buff;
  }

  void flush() {
    if (!cache::WylandDiskModuleBuffer.empty()) {
      handle.flush();
      auto bytes = compile_cache();
      handle.write((char*)bytes.data(), bytes.size());
      cache::WylandDiskModuleBuffer.clear();
    }
    
    handle.flush();
  }

  void write(wuint c) {
    cache::WylandDiskModuleBuffer.push_back(c);
    if (cache::WylandDiskModuleBuffer.size() == 256) flush();
  }

  void read() {
    if (handle.eof()) {
      last_get = weof;
      return;
    }

    handle.read(reinterpret_cast<char*>(&last_get), sizeof(last_get));
  }

public:
  wbool init() override {
    handle.open(path, std::ios::binary);
    if (!handle) {
      disk_error(typeid(this).name() + std::string(__func__), "unable to open disk file stream");
      return wyland_false;
    }
    
    handle.seekg(0);
    handle.seekp(0);
    return wyland_true;
  }

  std::string name() override { return typeid(this).name(); }

  void shutdown() override {
    flush();
    handle.close();
    // variables in the cache namespace are cleared by wyland_exit(int) function.
  }

  void send_data(wulong data) override {

  }

  wulong receive_data() override {

  }
};

WYLAND_END