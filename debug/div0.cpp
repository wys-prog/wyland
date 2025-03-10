#include "libs/asm.hpp"

int main() {
  #define libHandle 60
  #define funHandle 59
  #define returnVal 50

  BinarySerializer Serializer {
    // Load the librarie.
    load(48, 64, SYSTEM_SEGMENT_START + 48), // 11
    load(49, 8, 3),                          // 4
    interrupt(ops::sldlib),                  // 2

    // Save the handle
    mov(libHandle, returnVal),               // 3

    // Load the function
    load(48, 64, SYSTEM_SEGMENT_START + 51), // 11
    load(49, 8, 8),                          // 4
    mov(50, libHandle),                      // 3
    interrupt(ops::sldlcfun),                // 2

    // Save the handle
    mov(funHandle, returnVal),               // 3

    // Call the function
    mov(48, funHandle),                      // 3
    interrupt(ops::scfun),                   // 2

    db(0xFF), 
    'l', 'i', 'b',
    'f', 'u', 'n', 'c', 't', 'i', 'o', 'n',
  };

  std::ofstream file("divisionby0.bin");
  file.write((char*)Serializer.copy().data(), Serializer.copy().size());
  file.close();

  return 0;
}