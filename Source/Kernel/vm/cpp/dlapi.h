#ifndef DYNLOADER_H
#define DYNLOADER_H

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    typedef HMODULE LibHandle;
#else
    #include <dlfcn.h>
    typedef void* LibHandle;
#endif

#include <stdio.h>
typedef uint64_t (*FuncType)(uint8_t *);


LibHandle dynlib_open(const char *libPath) {
  if (!libPath)
    return NULL;

#if defined(_WIN32) || defined(_WIN64)
  LibHandle handle = LoadLibrary(libPath);
  if (!handle) {
    printf("Unable to load: %s\n", libPath);
  }
#else
  LibHandle handle = dlopen(libPath, RTLD_LAZY);
  if (!handle) {
    printf("Error: %s\n", dlerror());
  }
#endif

  return handle;
}

FuncType dynlib_get_function(LibHandle libHandle, const char *funcName) {
  if (!libHandle || !funcName)
    return nullptr;

#if defined(_WIN32) || defined(_WIN64)
  FuncType func = (FuncType)GetProcAddress(libHandle, funcName);
#else
  FuncType func = (FuncType)dlsym(libHandle, funcName);
#endif

  if (!func) {
    printf("Unable to find function: %s\n", funcName);
  }

  return func; 
}


void dynlib_close(LibHandle libHandle) {
  if (!libHandle)
    return;

#if defined(_WIN32) || defined(_WIN64)
  FreeLibrary(libHandle);
#else
  dlclose(libHandle);
#endif
}
#endif