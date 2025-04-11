/* Wyland Standard C Library */

#include <stdio.h>
#include <stdlib.h>

#include "wyland-runtime/wylrt.h"

wyland_extern(wyland_putc, flags) {
  // dmm0 -> (char) to write
  // qmm0 -> (void*) handle

  putc(*flags->regspointer->r32[0], (FILE*)*flags->regspointer->r64[0]);
}

wyland_extern(wyland_readc, flags) {
  // qmm0 -> (void*) handle

  *flags->regspointer->r32[0] = (wyland_uint)getc((FILE*)*flags->regspointer->r64[0]);
}

wyland_extern(wyland_open, flags) {
  // qmm0 -> (char*) name
  // qmm1 -> size_t  len
  // qmm2 -> (mode)  mode

  // returns:
  //  - WYLAND_NULL if the handle isn't openned 
  //  - a pointer to the handle, if the handle is opnned.

  char *name = (char*)malloc(*flags->regspointer->r64[1]);
  for (size_t i = 0; i < *flags->regspointer->r64[1]; i++) {
    name[i] = flags->segstart[i];
  }

  const char *mode;
  switch (*flags->regspointer->r64[2]) {
    case 0: mode = "r"; break;
    case 1: mode = "w"; break;
    case 2: mode = "a"; break;
    case 3: mode = "r+"; break;
    case 4: mode = "w+"; break;
    case 5: mode = "a+"; break;
    case 6: mode = "rb"; break;
    case 7: mode = "wb"; break;
    case 8: mode = "ab"; break;
    case 9: mode = "r+b"; break;
    case 10: mode = "w+b"; break;
    case 11: mode = "a+b"; break;
    default:
      *flags->regspointer->r64[0] = (wyland_ulong)WYLAND_NULL;
      free(name);
      return;
  }

  FILE *handle = fopen(name, mode);
  if (!handle) {
    *flags->regspointer->r64[0] = (wyland_ulong)WYLAND_NULL;
  } else {
    *flags->regspointer->r64[0] = (wyland_ulong)handle;
  }

  free(name);
}

wyland_extern(wyland_close, flags) {
  // qmm0 -> (void*)handle 
  fclose((FILE*)*flags->regspointer->r64[0]);
}

wyland_extern(wyland_get_stdout, flags) {
  // Returns a pointer to stdout
  printf("[c]: writing STDOUT's handle in QMM0\n");
  *flags->regspointer->r64[0] = (wyland_ulong)stdout;
}

wyland_extern(wyland_get_stdin, flags) {
  // Returns a pointer to stdin
  *flags->regspointer->r64[0] = (wyland_ulong)stdin;
}

wyland_extern(wyland_get_stderr, flags) {
  // Returns a pointer to stderr
  *flags->regspointer->r64[0] = (wyland_ulong)stderr;
}
