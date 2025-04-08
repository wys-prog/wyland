#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <sstream>

#ifdef _WIN32
  #include <windows.h>
  #define LIB_EXT ".dll"
#else
  #include <dlfcn.h>
  #ifdef __APPLE__
    #define LIB_EXT ".dylib"
  #else
    #define LIB_EXT ".so"
  #endif
#endif

#include "regs.hpp"
#include "wyland-runtime/wylrt.h"

#ifdef _WIN32
#define DynamicLibraryLoad(x) LoadLibraryA(x)
#define DynamicLibraryError() GetLastError()
#define DynamicLibraryFunc(h, n) GetProcAddress(static_cast<HMODULE>(h), n)
#define DynamicLibraryFree(h) FreeLibrary(static_cast<HMODULE>(h))
#else 
#define DynamicLibraryLoad(x) dlopen(x, RTLD_LAZY)
#define DynamicLibraryError() dlerror()
#define DynamicLibraryFunc(h, n) dlsym(h, n) 
#define DynamicLibraryFree(h) dlclose(h);
#endif // WIN ?

namespace wylma {
  namespace wyland {

    namespace libcallc {
      class DynamicLibrary {
        public:
        using FunctionType = void(*)(arg_t*);
        
        explicit DynamicLibrary(const std::string& libName) {
          loadLibrary(libName.c_str());
        }
        
        explicit DynamicLibrary() = default;
        
        ~DynamicLibrary() {
          unloadLibrary();
        }
        

        void loadLibrary(const char* libraryPath) {
          std::stringstream errorMsg;
          
          #ifdef _WIN32
          handle = LoadLibraryA(libraryPath);
          if (!handle) {
            errorMsg << "Failed to load library: " << libraryPath 
                    << " (Error code: " << GetLastError() << ")";
            throw std::runtime_error(errorMsg.str());
          }
          #else
          handle = dlopen(libraryPath, RTLD_LAZY);
          if (!handle) {
            errorMsg << "Failed to load library: " << libraryPath 
                    << " (Error: " << dlerror() << ")";
            throw std::runtime_error(errorMsg.str());
          }
          #endif
        }

        FunctionType loadFunction(const char* functionName) {
          std::stringstream errorMsg;
          
          if (!handle) {
            errorMsg << "Library is not loaded.";
            throw std::runtime_error(errorMsg.str());
          }

          #ifdef _WIN32
          FunctionType func = (FunctionType)(GetProcAddress(static_cast<HMODULE>(handle), functionName));
          #else
          FunctionType func = reinterpret_cast<FunctionType>(dlsym(handle, functionName));
          #endif

          if (!func) {
            errorMsg << "Failed to load function: " << functionName;
            #ifndef _WIN32
            errorMsg << " (Error: " << dlerror() << ")";
            #endif
            throw std::runtime_error(errorMsg.str());
          }

          return func;
        }

        
        void unloadLibrary() {
          if (handle) {
            #ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle));
            #else
            dlclose(handle);
            #endif
            handle = nullptr;
          }
        }
        
        private:
        void* handle = nullptr;
      };
    }

    typedef boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> linkedfn_array;
  }
}