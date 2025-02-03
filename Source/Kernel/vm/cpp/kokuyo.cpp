#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "kokuyo.hpp"
#include "std.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

class DynamicLibraryManager {
private:
  std::unordered_map<std::string, void *> libraries;

public:
  size_t size() const { return libraries.size(); }

  bool loadLibrary(const std::string &path) {
    if (libraries.find(path) != libraries.end()) {
      std::cerr << "Library already loaded: " << path << std::endl;
      return false;
    }

    void *handle = nullptr;
#if defined(_WIN32)
    handle = LoadLibraryA(path.c_str());
#else
    handle = dlopen(path.c_str(), RTLD_LAZY);
#endif

    if (!handle) {
      std::cerr << "Failed to load library: " << path << std::endl;
      return false;
    }

    libraries[path] = handle;
    return true;
  }

  void unloadLibrary(const std::string &path) {
    auto it = libraries.find(path);
    if (it != libraries.end()) {
#if defined(_WIN32)
      FreeLibrary(static_cast<HMODULE>(it->second));
#else
      dlclose(it->second);
#endif
      libraries.erase(it);
    }
  }

  void unloadAll() {
    for (auto &lib : libraries) {
#if defined(_WIN32)
      FreeLibrary(static_cast<HMODULE>(lib.second));
#else
      dlclose(lib.second);
#endif
    }
    libraries.clear();
  }

  template <typename FuncType>
  std::function<FuncType> getFunction(const std::string &path,
                                      const std::string &functionName) {
    auto it = libraries.find(path);
    if (it == libraries.end()) {
      std::cerr << "Library not loaded: " << path << std::endl;
      return nullptr;
    }

#if defined(_WIN32)
    auto funcPtr = reinterpret_cast<FuncType *>(
        GetProcAddress(static_cast<HMODULE>(it->second), functionName.c_str()));
#else
    auto funcPtr =
        reinterpret_cast<FuncType *>(dlsym(it->second, functionName.c_str()));
#endif

    if (!funcPtr) {
      std::cerr << "Failed to find function: " << functionName << std::endl;
      return nullptr;
    }

    return std::function<FuncType>(funcPtr);
  }

  ~DynamicLibraryManager() { unloadAll(); }
};

int main(int argc, char const *argv[]) {
  kokuyo::kokuyoVM vm;
  
  return 0;
}
