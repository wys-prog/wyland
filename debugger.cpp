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

uint8_t nop = 0, 
lea = 1,
load = 2, 
store = 3, 
mov = 4, 
add = 5, 
sub = 6, 
mul = 7, 
odiv = 8, 
mod = 9, 
jmp = 10, 
je = 11, 
jne = 12, 
jl = 13, 
jg = 14, 
jle = 15, 
jge = 16,
cmp = 17,
xint = 18;

uint8_t writec = 0, 
writerc = 1, 
readc = 2, 
csystem = 3, 
callec = 4, 
startt = 5,
pseg = 6;

template <typename T>
inline uint8_t* to_bin(const T &__T) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  uint8_t *buff = new uint8_t[sizeof(T)];

  for (size_t i = 0; i < sizeof(T); i++) {
    buff[i] = (__T >> ((sizeof(T) - 1 - i) * 8)) & 0xFF;
  }

  return buff;
}

class BinarySerializer {
private:
  std::vector<uint8_t> binaryData;

public:
  template<typename... Args>
  BinarySerializer(Args&&... args) {
    (serialize(std::forward<Args>(args)), ...);
  }

  // Sérialisation générique (big-endian)
  template<typename T>
  void serialize(const T& value) {
    T be_value = to_big_endian(value);
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&be_value);
    binaryData.insert(binaryData.end(), ptr, ptr + sizeof(T));
  }

  // Conversion en big-endian
  template<typename T>
  T to_big_endian(T value) {
    T result = T();
    uint8_t* src = reinterpret_cast<uint8_t*>(&value);
    uint8_t* dest = reinterpret_cast<uint8_t*>(&result);
    for (size_t i = 0; i < sizeof(T); i++) {
      dest[i] = src[sizeof(T) - 1 - i];
    }
    return result;
  }

  // Spécialisation pour std::string
  void serialize(const std::string& str) {
    uint32_t len = str.size();
    serialize(len);
    binaryData.insert(binaryData.end(), str.begin(), str.end());
  }

  // Retourne les données binaires
  const std::vector<uint8_t>& getBinaryData() const {
    return binaryData;
  }

  // Affichage des données en hexadécimal
  void printHex() const {
    for (uint8_t byte : binaryData) {
      printf("%02X ", byte);
    }
    std::cout << std::endl;
  }
};

int main() {
  // Address of the system segment: 0x00, 0x00, 0x00, 0x00, 0x1F ,0x40, 0x00, 0x00
  BinarySerializer buff {
    // pseg(BEG, LEN, ORG);
    load, 32, 48, 0x1F, 0x40, 0x00, (0x00+42), // BEG
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

    // }
  };
 

  std::ofstream out("out.bin");
  out.write((const char*)buff.getBinaryData().data(), buff.getBinaryData().size());

  
  out.close();
  return 0;
}