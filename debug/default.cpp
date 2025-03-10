#include "libs/asm.hpp"

int main() {
  #define counter  uint8_t(61)
  #define rv       uint8_t(50)

  #define PRINT_AD   SYSTEM_SEGMENT_START + 0x01
  #define PRINT_SIZE 28
  #define MAIN_BEG   PRINT_AD + PRINT_SIZE
  // KEYBOARD Beg in dec:   419430400
  //              in hex: 0x19000000
  #define MESSAGE_DEFAULT "No input provided. Defaulting to the basic shell.\n"

  BinarySerializer buff {
    // Print function
    // print(const char* ptr, uint64 len);
    #define _start()
      jmp(MAIN_BEG), 

    #define print(b,l) lea(0, b), load(49, 64, l) 
      #define ptr 48
      #define len 49

      load(2, 8, (uint8_t)1), 
      
      movad(0, ptr),
      interrupt(ops::writec), 

      add(ptr, 2), 
      add(counter, 2), 
      cmp(counter, 49),
      jne(PRINT_AD+1),
      #undef ptr
      #undef len
      ret, 
    

    #define main()
      load(63, 64, (uint64_t)MAIN_BEG+11+21),
      print(MAIN_BEG, (uint64_t)sizeof(MESSAGE_DEFAULT)-1), 
    
    db(0xFF),
    MESSAGE_DEFAULT,
  };

  

  std::ofstream out("default.bin");
  out.write((const char*)buff.getBinaryData().data(), buff.getBinaryData().size());
  
  out.close();
  return 0;
}