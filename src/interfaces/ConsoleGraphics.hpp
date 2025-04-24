#pragma once

#include <cstdio>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "interface.hpp"

WYLAND_BEGIN

class WylandConsoleGraphicsModule : public IWylandGraphicsModule {
private:
  wulong last_flag = 0;

public:
  void send_data(wulong data) override {
    auto bytes = to_bin_v<wulong>(data);
    bytes.push_back(0x00);
    std::fwrite(bytes.data(), 1, bytes.size(), stdout);
    std::fflush(stdout);
  }

  wulong receive_data() override { return last_flag; }
  std::string name() override { return typeid(this).name(); }
  // That's the simplest GPU man..
};

WYLAND_END