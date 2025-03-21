#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>

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
namespace wylma {
  namespace wyland {

    namespace libcallc {
      struct arg_t {
        uint8_t    *segstart;
        uint32_t    seglen; /* Use 32bits since 512MB is under 32 bits's max. */
        uint8_t    *keyboardstart;
        reg_t      *regspointer;
      };
      
      class DynamicLibrary {
        public:
        using FunctionType = void(*)(arg_t*);
        
        explicit DynamicLibrary(const std::string& libName) {
          std::string fullPath = libName + LIB_EXT;
          loadLibrary(fullPath.c_str());
        }
        
        explicit DynamicLibrary() = default;
        
        ~DynamicLibrary() {
          unloadLibrary();
        }
        
        FunctionType loadFunction(const char* functionName) {
          if (!handle) {
            throw std::runtime_error("Library is not loaded.");
          }
          
          #ifdef _WIN32
          FunctionType func = (FunctionType)(GetProcAddress(static_cast<HMODULE>(handle), functionName));
          #else
          FunctionType func = reinterpret_cast<FunctionType>(dlsym(handle, functionName));
          #endif
          
          if (!func) {
            throw std::runtime_error("Failed to load function: " + std::string(functionName));
          }
          
          return func;
        }
        
        void loadLibrary(const char* libraryPath) {
          #ifdef _WIN32
          handle = LoadLibraryA(libraryPath);
          #else
          handle = dlopen(libraryPath, RTLD_LAZY);
          #endif
          
          if (!handle) {
            throw std::runtime_error("Failed to load library: " + std::string(libraryPath));
          }
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
  }
  }