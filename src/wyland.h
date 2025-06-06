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
# define COMPILER_NAME "Unknown compiler"
#endif

#ifdef JAVA
# define WYLAND_NAME "Java Shit" 
/* We're coding in JS, Not JavaScript, but Java Shit !! */
#else
# ifdef __WYLAND_DEBUG__
#  define WYLAND_NAME    "Wyland wy:2.7 Debug"
# else
#  define WYLAND_NAME    "Wyland wy:2.7"
# endif // ? Debug ?
#endif // Java ?

#define WYLAND_BUILD_NAME "Wyland Build " WYLAND_VERSION_MAJOR_STR "." WYLAND_VERSION_MINOR_STR " - Sequoia - " COMPILER_NAME

#ifndef WYLAND_VERSION_UINT32
# define WYLAND_VERSION_UINT32 1
#endif // WYLAND_VERSION_UINT32 ?

#define WYLAND_SUPPORT_WTARG64 1
#define WYLAND_SUPPORT_WTARG32 0
#define WYLAND_SUPPORT_WTARGMARCH 0
#define WYLAND_SUPPORT_WTARGFAST 0

#ifndef ___WYLAND_BUILD_SYSTEM_BUILD_COUNT
# define ___WYLAND_BUILD_SYSTEM_BUILD_COUNT 0
#endif // ___WYLAND_BUILD_SYSTEM_BUILD_COUNT ?

#ifndef __WYLAND_BUILD_MODE
#define WYLAND_BUILD_MODE "unknown"
#else
#define WYLAND_BUILD_MODE __WYLAND_BUILD_MODE
#endif

#endif // __WYLAND_H__