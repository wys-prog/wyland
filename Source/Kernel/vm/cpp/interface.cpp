// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "kokuyo.hpp"
#include "exportapi.h"

typedef struct kokuyo_vm {
  kokuyo vm_instance;
} kokuyo_vm;

extern "C" EXPORT_API kokuyo_vm *kokuyo_create() {
  kokuyo_vm *new_vm = (kokuyo_vm *)malloc(sizeof(kokuyo_vm));
  if (new_vm != NULL) {
    new_vm->vm_instance = kokuyo();
  }
  return new_vm;
}

extern "C" EXPORT_API void kokuyo_destroy(kokuyo_vm *vm) {
  if (vm != NULL) {
    free(vm);
  }
}

extern "C" EXPORT_API void kokuyo_invoke(kokuyo_vm *vm, const char *_disk) {
  std::ifstream file(_disk);

  if (!file) throw std::invalid_argument("No such file: " + std::string(_disk));

  std::vector<uint8_t> disk;

  while (!file.eof()) {
    char buff[1] = {0};
    file.read(buff, 1);

    disk.push_back(buff[0]);
  }

  disk.shrink_to_fit();

  vm->vm_instance.invoke(disk);
}