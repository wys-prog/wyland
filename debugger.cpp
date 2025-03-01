#include <bits/stdc++.h>

enum eins : uint8_t {
  nop, 
  lea,
  load, 
  store, 
  mov, 
  add, 
  sub, 
  mul, 
  odiv, 
  mod, 
  jmp, 
  je, 
  jne, 
  jl, 
  jg, 
  jle, 
  jge,
  cmp,
  xint, 
};

int main() {
  std::ofstream file("out.bin");
  
  uint8_t buffer[512] = {
    /*load, 8, 6, 0xFE, 
    load, 8, 1, 0x01, 
    add, 06, 01, 
    0x00, 
    0x00, 
    load, 8, 2, 0xFF, 
    jne, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0xFF*/
    
    load, 8, 06, 0x02, 
    load, 8, 01, 0xFF, // (0xFF = 255)
    load, 8, 02, 0x05, 
    add, 06, 02, 
    cmp, 06, 01,
    je, 0,0,0,0,0,0,0xFF,0xFA,
    jmp, 0,0,0,0,0,0,0,0,0,
    0xFF,
  };

  buffer[510] = 0xFF;
  file.write((char*)buffer, sizeof(buffer));
  file.close();
  return 0;
}