#ifndef __WYLAND_RUNTIME_LIB__
#define __WYLAND_RUNTIME_LIB__

#include <stdint.h>

typedef struct {
  char    *what;
  char    *name;
  char    *caller;
  uint64_t ip;
  uint64_t thread;
  uint64_t *segmeg;
  uint64_t *segmend;
  uint64_t  segsize;
} wylrterror;

#endif // __WYLAND_RUNTIME_LIB__