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
#include "../wmmio.hpp"

#include "winface.h"

WYLAND_BEGIN

class IWylandGraphicsModule : public WylandMMIOModule {
public:
  virtual ~IWylandGraphicsModule() {}
  virtual wbool init(wint width, wint height, const std::string &title) { return wyland_true; }
  virtual void shutdown() {}
  virtual void process(wfloat delta) {} // Godot style !
  virtual void render() {}
  virtual wbool should_close() { return wyland_false; }
  virtual std::string name() { return typeid(this).name(); }
};

class GraphicsModuleException : public runtime::wyland_runtime_error {
public:
  GraphicsModuleException(const std::string &what, const std::string &from) 
    : runtime::wyland_runtime_error(what.c_str(), "GraphicsModule Exception", from.c_str(), typeid(this).name(), 0, 0, NULL, NULL, 0)
  {}
};


WYLAND_END