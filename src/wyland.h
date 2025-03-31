#ifndef __WYLAND_H__
#define __WYLAND_H__

#if defined(__clang__)
# define COMPILER_NAME "Clang"
#elif defined(__GNUC__) || defined(__GNUG__)
# define COMPILER_NAME "GCC"
#elif defined(_MSC_VER)
# define COMPILER_NAME "MSVC"
#elif defined(__INTEL_COMPILER)
# define COMPILER_NAME "Intel ICC/ICPC"
#elif defined(__EMSCRIPTEN__)
# define COMPILER_NAME "Emscripten"
#elif defined(__MINGW32__) || defined(__MINGW64__)
# define COMPILER_NAME "MinGW"
#else
# define COMPILER_NAME ""
#endif

#define WYLAND_NAME    "Wyland wy:2.4"
#define WYLAND_VERSION "1.2.4"
#define WYLAND_BUILD   "Wyland Build 1.2 - Tokyo - " COMPILER_NAME
#define WYLAND_VERSION_UINT32 1

#define WYLAND_SUPPORT_WTARG64 1
#define WYLAND_SUPPORT_WTARG32 0
#define WYLAND_SUPPORT_WTARGMARCH 0
#define WYLAND_SUPPORT_WTARGFAST 0

#endif // __WYLAND_H__