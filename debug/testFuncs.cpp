#include "../wylrt.h"
#include "../libcallc.hpp"

extern "C" void function(libcallc::arg_t *args) {
  args->regspointer->set(0, 'A');
}