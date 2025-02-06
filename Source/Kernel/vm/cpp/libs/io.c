#include <stdint.h>
#include <stdio.h>

#include "export.h"

EXPORT_API uint64_t write(uint8_t *argv) {
  return printf((char*)argv); /* Use the printf function. Easy, and fast. */
}

/* Todo (general project)
  I/O manipulation
  threading 
  C++ try/catch/throw system
  dynamic allocation
 */