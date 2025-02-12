#include <stdio.h>
#include <stdint.h>

__attribute__((visibility("default"))) 
uint64_t func(uint64_t argc, uint8_t *argv) {
  printf("Hello, world.\n");
  return 0;
}
