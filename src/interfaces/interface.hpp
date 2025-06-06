#pragma once

/* WYLAND 
    Wys's Virtual Wyland Machine
    Version: Idk 
    This file: an interface for graphicale (and much more) interfaces.
    Have fun ?
*/

#include <string>
#include <sstream>
#include <iostream>

#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wylrt.hpp"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wlongfloats.h"

#include "../wmmbase.hpp"
#include "../wmmio.hpp"

#include "winface.h"

WYLAND_BEGIN

class IWylandGraphicsModule : public WylandMMIOModule {
public:
  virtual ~IWylandGraphicsModule() {}
  virtual wbool init(wint, wint, const std::string &) { return wyland_true; }
  virtual wbool init() override { return wyland_true; }
  virtual void process(wlongfloat) {} // Godot style !
  virtual void render() {}
  virtual wbool should_close() { return wyland_false; }
  std::string name() override { return typeid(this).name(); }
  virtual std::ostream *get_stream() { return &std::cout; }
};

class GraphicsModuleException : public runtime::wyland_runtime_error {
public:
  GraphicsModuleException(const std::string &what, const std::string &from) 
    : runtime::wyland_runtime_error(what.c_str(), "GraphicsModule Exception", from.c_str(), typeid(this).name(), 0, 0, NULL, NULL, 0)
  {}
};

class InvalidGraphicsModuleStreamException : public GraphicsModuleException {
public:
  InvalidGraphicsModuleStreamException(const std::string &what, const std::string &from) 
    : GraphicsModuleException(what, from)
  {}
};

class HorribleGraphicsModule : public IWylandGraphicsModule {
private:
public:
  void process(wlongfloat delta) override {
    std::cout << "delta: " << longfloat_to_str(delta) << std::endl;
  }
};

WYLAND_END