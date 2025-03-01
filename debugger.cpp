#include <bits/stdc++.h>

enum eins {
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

int main(int argc, char const *argv[]) {
  std::ofstream file("out.bin");
  
  uint8_t buffer[512] = {
    load, 8, 01, 0xFF, // (0xFF = 255)
    load, 8, 02, 0x01, 
    add, 03, 02, 
    cmp, 03, 01, 
    jne, 0,0,0,0,0,0,0,0,0,
    0xFF, 
  };

  buffer[510] = 0xFF;
  file.write((char*)buffer, sizeof(buffer));
  file.close();
  return 0;
}