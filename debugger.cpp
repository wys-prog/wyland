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
  writec, // OK
  readc,  // OK
  writerc, // OK
  startt,  // OK
  csystem, // OK
  // writedisk, not in the standard 25 of KokuyoVM... 
  // readdisk,  not in the standard 25 of KokuyoVM... 
  // getdiskif, not in the standard 25 of KokuyoVM... 
  callec, // OK
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
  auto systemHBEG = SYSTEM_SEGMENT_START;
  // Address of BEG: 0x00, 0x00, 0x00, 0x00, 0x1F ,0x40, 0x00, 0x00
  uint8_t buff[] = {
    load, 8, 00, '1', 
    load, 8, 02, '2', 
    store, 8, 02, /* ADDRESSE */ 0x00, 0x00, 0x00, 0x00, 0x1F ,0x40, 0x00, 0x00,
    lea, 
    0xFF
  };
 

  std::ofstream out("out.bin");
  out.write((const char*)buff, sizeof(buff));

  
  out.close();
  return 0;
}