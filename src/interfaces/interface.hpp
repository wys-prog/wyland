#pragma once

/* WYLAND 
    Wys's Virtual Wyland Machine
    Version: Idk 
    This file: an interface for graphicale (and much more) interfaces.
    Have fun ?
*/

#include <string>

#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wylrt.hpp"
#include "../wyland-runtime/wfloats.h"

#include "../wmmbase.hpp"

WYLAND_BEGIN

class IWylandGraphicsModule {
protected:
  wuchar *memory_begin = 0; /* That memory is different than @memory */
  wulong  memory_size = 0;

public:
  virtual ~IWylandGraphicsModule() {}
  virtual wbool init(wint width, wint height, const std::string &title) { return wyland_true; }
  virtual void shutdown() {}
  virtual void process(wfloat delta) {} // Godot style  !
  virtual void render() {}
  virtual wbool should_close() { return wyland_false; }
  virtual std::string name() { return "WylandGraphicsModuleTemplate"; }
};

class GraphicsModuleException : public runtime::wyland_runtime_error {
public:
  GraphicsModuleException(const std::string &what, const std::string &from) 
    : runtime::wyland_runtime_error(what.c_str(), "GraphicsModule Exception", from.c_str(), typeid(this).name(), 0, 0, NULL, NULL, 0)
  {}
};


WYLAND_END