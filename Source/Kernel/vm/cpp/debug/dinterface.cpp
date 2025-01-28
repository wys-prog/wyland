// コクウキョウ

// Kokūkyō - dobuyo for Wyland Project
// Modified version of dobuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Debuyo.hpp"
#include "../exportapi.h"

typedef struct dobuyo_vm {
  Dobuyo vm_instance;
} dobuyo_vm;

extern "C" EXPORT_API dobuyo_vm *dobuyo_create() {
  dobuyo_vm *new_vm = (dobuyo_vm *)malloc(sizeof(dobuyo_vm));
  if (new_vm != NULL) {
    return nullptr;
  }
  return new_vm;
}

extern "C" EXPORT_API void dobuyo_destroy(dobuyo_vm *vm) {
  if (vm != NULL) {
    free(vm);
  }
}

extern "C" EXPORT_API void dobuyo_invoke(dobuyo_vm *vm, const char *_disk) {
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

extern "C" EXPORT_API uint64_t dobuyo_get_ip(dobuyo_vm *vm) {
  return vm->vm_instance.get_ip();
}

extern "C" EXPORT_API uint64_t dobuyo_get_memory(dobuyo_vm *vm) {
  return vm->vm_instance.get_memory();
}

extern "C" EXPORT_API const char *dobuyo_get_registers(dobuyo_vm *vm) {
  return vm->vm_instance.get_registers().c_str();
}

extern "C" EXPORT_API const char *dobuyo_get_callstack(dobuyo_vm *vm) {
  return vm->vm_instance.get_callstack().c_str();
}

extern "C" EXPORT_API const char *dobuyo_get_stack(dobuyo_vm *vm) {
  return vm->vm_instance.get_stack().c_str();
}

extern "C" EXPORT_API bool dobuyo_is_halted(dobuyo_vm *vm) {
  return vm->vm_instance.is_halted();
}

extern "C" EXPORT_API void dobuyo_step(dobuyo_vm *vm) {
  vm->vm_instance.step();
}

extern "C" EXPORT_API void dobuyo_end(dobuyo_vm *vm) {
  vm->vm_instance.end();
}