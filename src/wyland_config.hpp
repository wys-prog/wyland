#pragma once

#ifdef WYLAND_STACKTRACE
//#include "boost/stacktrace.hpp"
#define WYLAND_STACKTRACE_ENABLE (1)
#define WYLAND_GET_STACKTRACE "-- SOON --"
#else
#define WYLAND_STACKTRACE_ENABLE (0)
#define WYLAND_GET_STACKTRACE "-- not avaible on this build --"
#endif // ? WYLAND_STACK_TRACE