#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include <string>

#include "wmmbase.hpp"
#include "wyland-runtime/wylrt.hpp"

#include "libcallc.hpp"

#include "cache.hpp"
#include "def/wiodef.hpp"

WYLAND_BEGIN

class WylandMMIOModule;

typedef wbool (*EMMIOFuncSignBool)(void);
typedef void (*EMMIOFunc)(void);
typedef const char *(*EMMIOFuncSignStr)(void);
typedef void (*EMMIOFuncArgU64)(wulong);
typedef void (*EMMIOFuncArgByte)(wbyte);
typedef wulong (*EMMIOFuncSignU64)(void);
typedef wuint (*EMMIOFuncSignU32)(void);
typedef wbyte (*EMMIOFuncSignByte)(void);

#ifndef ___WYLAND_BUILD_CACHE___

class IWylandMMIOExternalModule : public WylandMMIOModule {
public:
  EMMIOFuncSignBool Einit = nullptr;
  EMMIOFunc Eshutdown = nullptr;
  EMMIOFuncSignStr Ename = nullptr;
  EMMIOFuncArgU64 Esend_data = nullptr;
  EMMIOFuncSignU64 Ereceive_data = nullptr;

  wbool init() override { return Einit(); }
  void shutdown() override { Eshutdown(); }
  std::string name() override { return typeid(this).name(); }
  void send_data(wulong data) override { Esend_data(data); }
  wulong receive_data() override { return Ereceive_data(); }
  // Destructor is not needed in this one, because... WE USE wylma::wyland::cache !!!! 
};

inline WylandMMIOModule *loadIExternalMMIOModule(const std::string &path) {

  const auto handle = DynamicLibraryLoad(path.c_str());
  if (!handle) {
    std::cerr << "Failed to load library: " << path << std::endl;
    return nullptr;
  }

  auto module = new IWylandMMIOExternalModule();
  module->Einit = reinterpret_cast<EMMIOFuncSignBool>(DynamicLibraryFunc(handle, "Einit"));
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

  cache::WylandMMIOModuleHandles.push_back(handle);

  return module;
}

#endif // ___WYLAND_BUILD_CACHE___

class MMIOModuleException : public runtime::wyland_runtime_error {
public:
  MMIOModuleException(const std::string &what, const std::string from) 
    : runtime::wyland_runtime_error(what.c_str(), "MMIO Module Exception", from.c_str(), 
    typeid(this).name(), -1, 0, NULL, NULL, -1) {}
};

WYLAND_END