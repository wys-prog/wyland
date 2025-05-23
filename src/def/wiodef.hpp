#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include <string>

#include "../wmmbase.hpp"
#include "../wyland-runtime/wylrt.hpp"

#include "../libcallc.hpp"

WYLAND_BEGIN

class WylandMMIOModule {
private: /* NOTHING ! HAHAHAHAHA */
public:
  virtual wbool init() { return true; }
  virtual void shutdown() {}
  virtual std::string name() { return typeid(this).name(); }
  virtual void send_data(wulong) {} /* Only 64 bits/call. */
  virtual wulong receive_data() { return -1; }
  virtual ~WylandMMIOModule() {}
};

WYLAND_END