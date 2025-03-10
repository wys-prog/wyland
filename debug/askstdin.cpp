#include "libs/asm.hpp"

int main() {
  #define counter uint8_t(61)
  #define rv      uint8_t(50)
  // KEYBOARD Beg in dec:   419430400
  //              in hex: 0x19000000

  BinarySerializer buff {
    // Read from STDIN, and write the line into STDOUT
    interrupt(ops::reads), 
    lea(48, KEYBOARD_SEGMENT_START), 
    load(2, 8, (uint8_t)1), 
    
    movad(0, 48), 
    interrupt(ops::writec), 

    add(counter, 2), 
    cmp(counter, rv),
    add(48, 2),
    jne(SYSTEM_SEGMENT_START + 16),

    load(0, 8, '\n'), 
    interrupt(ops::writec), 

    db(0xFF)
  };

  

  std::ofstream out("out.bin");
  out.write((const char*)buff.getBinaryData().data(), buff.getBinaryData().size());
  
  out.close();
  return 0;
}