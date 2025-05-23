#pragma once

/* WYLAND 
    Wys's Virtual Wyland Machine
    Version: Idk 
    This file: an interface for graphicale (and much more) interfaces.
    Have fun ?
*/

#include <string>
#include <iostream>

#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wylrt.hpp"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wstreambase.h"

#include "../wmmbase.hpp"

#include "../libcallc.hpp"

#include "interface.hpp"

#include "cache.hpp"

WYLAND_BEGIN

typedef void (*IEGMFunc)(void);
typedef void (*IEGMFuncProcess)(wfloat);
typedef wbool (*IEGMFuncSignBool)(void);
typedef wbool (*IEGMFuncInit)(wint, wint, const char*);
typedef const char *(*IEGMFuncName)();
typedef wystream *(*IEGMFuncGetStream)();

#ifndef ___WYLAND_NOT_MAIN_BUILD___

/*namespace cache {
  extern std::vector<DynamicLibraryHandle> IExternalGraphicsModuleHandles{};
}*/

#endif // ___WYLAND_NOT_MAIN_BUILD___

class IExternalGraphicsModule : public IWylandGraphicsModule {
public:
  IEGMFunc          Eshutdown;
  IEGMFunc          Erender;
  IEGMFuncInit      Einit;
  IEGMFuncSignBool  Eshould_close;
  IEGMFuncProcess   Eprocess;
  IEGMFuncName      Ename;
  IEGMFuncGetStream Eget_stream;
  
  wbool init(wint width, wint height, const std::string &title) override {
    return Einit(width, height, title.c_str());
  }
  
  void shutdown() override { Eshutdown(); }
  void render() override { Erender(); }
  wbool should_close() override { return Eshould_close(); }
  std::string name() override { return Ename(); }
  std::ostream *get_stream() override {
    auto ptr = Eget_stream();
    if ((ptr->type != wstream_type_ostream && ptr->type != wstream_type_iostream) || !ptr) {
      throw InvalidGraphicsModuleStreamException("ptr->type is invalid (cannot write on it)", typeid(this).name() + "/"s + (__func__));
    }

    return (std::ostream*)ptr->stream;
  }
};

IExternalGraphicsModule *loadIExternalGraphicsModule(const std::string &path) {
  auto handle = DynamicLibraryLoad(path.c_str());
  if (!handle) {
    std::cerr << "Failed to load library: " << path << std::endl;
    return nullptr;
  }

  auto module = new IExternalGraphicsModule();
  module->Eshutdown = reinterpret_cast<IEGMFunc>(DynamicLibraryFunc(handle, "Eshutdown"));
  module->Erender = reinterpret_cast<IEGMFunc>(DynamicLibraryFunc(handle, "Erender"));
  module->Einit = reinterpret_cast<IEGMFuncInit>(DynamicLibraryFunc(handle, "Einit"));
  module->Eshould_close = reinterpret_cast<IEGMFuncSignBool>(DynamicLibraryFunc(handle, "Eshould_close"));
  module->Eprocess = reinterpret_cast<IEGMFuncProcess>(DynamicLibraryFunc(handle, "Eprocess"));
  module->Ename = reinterpret_cast<IEGMFuncName>(DynamicLibraryFunc(handle, "Ename"));
  module->Eget_stream = reinterpret_cast<IEGMFuncGetStream>(DynamicLibraryFunc(handle, "Eget_stream"));

  if (!module->Eshutdown || !module->Erender || !module->Einit || 
      !module->Eshould_close || !module->Eprocess || !module->Ename) {
    std::cerr << "Failed to load one or more symbols from: " << path << "what(): " << DynamicLibraryError() << std::endl;
    delete module;
    DynamicLibraryFree(handle);
    return nullptr;
  }

  cache::IExternalGraphicsModuleHandles.push_back(handle);

  return module;
}

WYLAND_END