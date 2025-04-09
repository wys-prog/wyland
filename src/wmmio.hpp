#pragma once

#include "wmmbase.hpp"
#include "wyland-runtime/wylrt.h"

WYLAND_BEGIN

class WylandMMIOModule {
private: /* NOTHING ! HAHAHAHAHA */
public:
  virtual wbool init() {}
  virtual void shutdown() {}
  virtual std::string name() { return typeid(this).name(); }
  virtual void send_data(uint64_t) {} /* Only 64 bits/call. */
  virtual uint64_t recive_data() { return -1; }
};

WYLAND_END