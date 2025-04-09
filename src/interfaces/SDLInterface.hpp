#pragma once

#include <string>

#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wylrt.hpp"
#include "../wyland-runtime/wfloats.h"

#include "../wmmbase.hpp"

#include "interface.hpp"

WYLAND_BEGIN

class IWylandBuiltInSDLGraphicsModule : public IWylandGraphicsModule {
public:
  ~IWylandBuiltInSDLGraphicsModule();
  wbool init(wint width, wint height, const std::string &title);
  void shutdown();
  void process(wfloat delta, wpacked_frame *frame); // Godot style  !
  void render();
  wbool should_close();
  std::string name();

};



WYLAND_END