#include <bits/stdc++.h>

constexpr std::size_t operator""_MB(unsigned long long size) {
  return size * 1024 * 1024;
}

constexpr std::size_t operator""_GB(unsigned long long size) {
  return size * 1024 * 1024 * 1024;
}

#define CODE_SEGMENT_SIZE 400_MB
#define HARDWARE_SEGMENT_SIZE 100_MB
#define SYSTEM_SEGMENT_SIZE 12_MB

#define CODE_SEGMENT_START 0
#define HARDWARE_SEGMENT_START (CODE_SEGMENT_START + CODE_SEGMENT_SIZE)
#define SYSTEM_SEGMENT_START (HARDWARE_SEGMENT_START + HARDWARE_SEGMENT_SIZE)

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

enum syscall : uint8_t {
  writec, 
  writerc, 
  readc, 
  csystem, 
  callec, 
  startt,
  pseg
};

template <typename T>
inline uint8_t* to_bin(const T &__T) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  uint8_t *buff = new uint8_t[sizeof(T)];

  for (size_t i = 0; i < sizeof(T); i++) {
    buff[i] = (__T >> ((sizeof(T) - 1 - i) * 8)) & 0xFF;
  }

  return buff;
}

int main() {
  // Address of the system segment: 0x00, 0x00, 0x00, 0x00, 0x1F ,0x40, 0x00, 0x00
  uint8_t buff[] = {
    // pseg(BEG, LEN, ORG);
    load, 32, 48, 0x1F ,0x40, 0x00, (0x00+42), // BEG
    load, 8, 49, 23, // LEN
    load, 8, 50, 0x00, // ORG
    xint, pseg,

    load, 8, 48, 0x00, 
    load, 16, 49, 0xFF, 0xFF, 
    load, 8, 01, 01, 
    xint, startt, 
    

    jmp, 0x00, 0x00, 0x00, 0x00, 0x1F ,0x40, 0x00, 0x00+uint8_t(32),

    0xFF, 
    // .data
    // .def(codeblock) {
      load, 8, 49, 6,
      xint, csystem, 
      0xFF,

      'e', 'c', 'h', 'o', ' ', 'e', 'n', 'd', 'i', 'n', 'g', ';', 
      'e', 'x', 'i', 't', 
    // }
  };
 

  std::ofstream out("out.bin");
  out.write((const char*)buff, sizeof(buff));

  
  out.close();
  return 0;
}