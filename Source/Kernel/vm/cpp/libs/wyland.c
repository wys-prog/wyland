#include <stdint.h>
#include <stdio.h>

#include "export.h"

EXPORT_API uint64_t write(uint64_t, uint8_t *argv) {
  return printf((char*)argv); /* Use the printf function. Easy, and fast. */
}

/* Other functions will may be inplemented in the system's lib folder.. This lib contains 
  basic function, and mainly a function to write string to the standard output. */