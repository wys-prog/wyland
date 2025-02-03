#ifndef ___KLANG_DLB_H___
#define ___KLANG_DLB_H___

#ifdef _WIN32
#include <windows.h>
typedef HMODULE DLibHandle;
#else
#include <dlfcn.h>
typedef void *DLibHandle;
#endif

#include <stdio.h>

static inline DLibHandle dlb_open(const char *path) {
  if (!path)
    return NULL;

#ifdef _WIN32
  return LoadLibraryA(path);
#else
  return dlopen(path, RTLD_LAZY);
#endif
}

static inline void dlb_close(DLibHandle handle) {
  if (!handle)
    return;

#ifdef _WIN32
  FreeLibrary(handle);
#else
  dlclose(handle);
#endif
}

static inline void *dlb_get_function(DLibHandle handle, const char *func_name) {
  if (!handle || !func_name)
    return NULL;

#ifdef _WIN32
  return (void *)GetProcAddress(handle, func_name);
#else
  return dlsym(handle, func_name);
#endif
}

#endif // ___KLANG_DLB_H___
