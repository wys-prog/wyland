#include "../wylrt.h"
#include "../libcallc.hpp"
#include "stdio.h"

extern "C" void function(libcallc::arg_t *args) {
  printf("Hello, from C, but called from a VM.");
  args->regspointer->set(0, 'A');
}