#pragma once

#include <string>

#include "wmmbase.hpp"
#include "wyland-runtime/wylrt.h"

#include "libcallc.hpp"

WYLAND_BEGIN

class WylandMMIOModule {
private: /* NOTHING ! HAHAHAHAHA */
public:
  virtual wbool init() { return true; }
  virtual void shutdown() {}
  virtual std::string name() { return typeid(this).name(); }
  virtual void send_data(uint64_t) {} /* Only 64 bits/call. */
  virtual uint64_t receive_data() { return -1; }
};

typedef wbool (*EMMIOFuncSingBool)(void);
typedef void (*EMMIOFunc)(void);
typedef const char *(*EMMIOFuncSignStr)(void);
typedef void (*EMMIOFuncArgU64)(uint64_t);
typedef uint64_t (*EMMIOFuncSignU64)(void);

class IWylandMMIOExternalModule : public WylandMMIOModule {
public:
  EMMIOFuncSingBool Einit;
  EMMIOFunc Eshutdown;
  EMMIOFuncSignStr Ename;
  EMMIOFuncArgU64 Esend_data;
  EMMIOFuncSignU64 Ereceive_data;

  wbool init() override { return Einit(); }
  void shutdown() override { Eshutdown(); }
  std::string name() override { return typeid(this).name(); }
  void send_data(uint64_t data) override { Esend_data(data); }
  uint64_t receive_data() override { return Ereceive_data(); }
};

WylandMMIOModule *loadIExternalMMIOModule(const std::string &path) {

  auto handle = DynamicLibraryLoad(path.c_str());
  if (!handle) {
    std::cerr << "Failed to load library: " << path << std::endl;
    return nullptr;
  }

  auto module = new IWylandMMIOExternalModule();
  module->Einit = reinterpret_cast<EMMIOFuncSingBool>(DynamicLibraryFunc(handle, "Einit"));
  module->Ename = reinterpret_cast<EMMIOFuncSignStr>(DynamicLibraryFunc(handle, "Ename"));
  module->Eshutdown = reinterpret_cast<EMMIOFunc>(DynamicLibraryFunc(handle, "Eshutdown"));
  module->Ereceive_data = reinterpret_cast<EMMIOFuncSignU64>(DynamicLibraryFunc(handle, "Ereceive_data"));
  module->Esend_data = reinterpret_cast<EMMIOFuncArgU64>(DynamicLibraryFunc(handle, "Esend_data"));

  if (!module->Einit || !module->Ename || !module->Eshutdown || 
      !module->Ereceive_data || !module->Esend_data) {
    std::cerr << "Failed to load one or more functions: " << DynamicLibraryError() << std::endl;
    DynamicLibraryFree(handle);
    delete module;
    return nullptr;
  }

  DynamicLibraryFree(handle);

  return module;
}

class MMIOModuleException : public runtime::wyland_runtime_error {
public:
  MMIOModuleException(const std::string &what, const std::string from) 
    : runtime::wyland_runtime_error(what.c_str(), "MMIO Module Exception", from.c_str(), 
    typeid(this).name(), -1, 0, NULL, NULL, -1) {}
};

WYLAND_END