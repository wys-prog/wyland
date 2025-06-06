#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <array>

#include "wmmbase.hpp"
#include "wmmio.hpp"
#include "wyland-runtime/wylrt.h"
#include "wyland-runtime/wylrt.hpp"

#include "filestream.hpp"

#include "cache.hpp"

WYLAND_BEGIN

#ifndef ___WYLAND_NOT_MAIN_BUILD___

/*namespace cache {
  // Each blocks are 4096 bytes, so our array contains 1024 bytes (4096 / sizeof(wuint))
  static constexpr size_t BLOCK_SIZE_BYTES = 4096;
  static constexpr size_t WUINTS_PER_BLOCK = BLOCK_SIZE_BYTES / sizeof(wuint);
  
  extern std::vector<wuint> WylandDiskModuleBuffer{};
  extern std::array<wuint, WUINTS_PER_BLOCK> ReadBlockBuffer{};
  extern size_t ReadBlockIndex = 0;
  extern size_t ReadBlockSize = 0;
}*/

#endif // ___WYLAND_NOT_MAIN_BUILD___

class IWylandDiskModule : public WylandMMIOModule {
private:
  w_dfstream &handle;

  void disk_error(const std::string &from, const std::string &what) {
    throw runtime::wyland_runtime_error(what.c_str(), "disk error", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0);
  }

  void flush() {
    if (!cache::WylandDiskModuleBuffer.empty()) {
      std::vector<wbyte> raw;
      for (const wuint val : cache::WylandDiskModuleBuffer) {
        auto bytes = to_bin_v(val); // big endian
        raw.insert(raw.end(), bytes.begin(), bytes.end());
      }

      handle.write(reinterpret_cast<const char*>(raw.data()), raw.size());
      if (!handle.is_open()) {
        disk_error("flush", "write failed");
      }

      cache::WylandDiskModuleBuffer.clear();
    }

    handle.flush();
  }

  bool read_block() {
    handle.read(reinterpret_cast<char*>(cache::ReadBlockBuffer.data()), BLOCK_SIZE_BYTES);
    cache::ReadBlockSize = static_cast<size_t>(handle.gcount()) / sizeof(wuint);
    cache::ReadBlockIndex = 0;

    return cache::ReadBlockSize > 0;
  }

  wuint read_next_wuint() {
    if (cache::ReadBlockIndex >= cache::ReadBlockSize) {
      if (!read_block()) {
        return weof;
      }
    }

    return cache::ReadBlockBuffer[cache::ReadBlockIndex++];
  }

  void write_wuint(wuint val) {
    cache::WylandDiskModuleBuffer.push_back(val);
    if (cache::WylandDiskModuleBuffer.size() >= WUINTS_PER_BLOCK) {
      flush();
    }
  }

public:
  wbool init() override {
    if (!handle.is_open()) {
      disk_error("init", "unable to open disk file stream");
      return wyland_false;
    }

    cache::WylandDiskModuleBuffer.clear();
    cache::ReadBlockIndex = 0;
    cache::ReadBlockSize = 0;

    return wyland_true;
  }

  std::string name() override { return typeid(this).name(); }

  void shutdown() override {
    flush();
    handle.close();
  }

  void send_data(wulong data) override {
    wuint info = static_cast<wuint>((data >> 32) & 0xFFFFFFFF);
    wuint val  = static_cast<wuint>(data & 0xFFFFFFFF);

    write_wuint(info);
    write_wuint(val);
  }

  wulong receive_data() override {
    wuint info = read_next_wuint();
    if (info == (wuint)weof) return static_cast<wulong>(weof);

    wuint val = read_next_wuint();
    if (val == (wuint)weof) return static_cast<wulong>(weof);

    wulong result = 0;
    result |= static_cast<wulong>(info) << 32;
    result |= static_cast<wulong>(val);

    return result;
  }

  IWylandDiskModule(w_dfstream &streaaaammmm) : handle(streaaaammmm) {}
};

WYLAND_END
