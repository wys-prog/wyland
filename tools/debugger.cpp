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

#define KEYBOARD_SEGMENT_START HARDWARE_SEGMENT_START
#define KEYBOARD_SEGMENT_SIZE  2_MB 

typedef struct {
  std::string name;
} ldef;

typedef struct {
  std::string name;
} lref;

namespace ops {
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
          xint = 18,
          loadat = 19, 
          ret    = 20,
          movad  = 21;

  uint8_t writec = 0, 
          writerc = 1, 
          readc = 2, 
          csystem = 3, 
          callec = 4, 
          startt = 5,
          pseg = 6,
          reads = 7, 
          sldlcfun = 8, 
          suldlib = 9;
}

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
  std::unordered_map<std::string, uint64_t> labels;
  std::vector<std::pair<size_t, std::string>> unresolved_labels;
  std::vector<uint8_t> binaryData;

  template<typename T>
  void serialize(const T& value) {
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      serialize(value()); // Appelle la fonction et sérialise son retour
    } else {
      T be_value = to_big_endian(value);
      const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&be_value);
      binaryData.insert(binaryData.end(), ptr, ptr + sizeof(T));
    }
  }

  template <typename T>
  void serialize(const std::vector<T> &vec) {
    for (const auto &e:vec) serialize(e);
  }

  void serialize(const ldef &def) {
    if (labels.find(def.name) != labels.end()) std::cerr << "Redefinition:\t" << def.name << std::endl;
    
    labels[def.name] = binaryData.size();
  }

  void serialize(const lref &ref) {
    if (labels.find(ref.name) != labels.end()) {
      serialize(labels[ref.name]);  // Si déjà défini, on sérialise l'adresse directement.
    } else {
      // On stocke la position actuelle et le nom du label à résoudre plus tard.
      unresolved_labels.push_back({binaryData.size(), ref.name});
      serialize(uint64_t(0));  // Placeholder temporaire.
    }
  }

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

public:
  template<typename... Args>
  BinarySerializer(Args&&... args) {
    (serialize(std::forward<Args>(args)), ...);
  }
  const std::vector<uint8_t>& getBinaryData() const {
    return binaryData;
  }

  std::vector<uint8_t> copy() {
    return std::vector<uint8_t>(binaryData);
  }

  void resolveLabels() {
    for (auto &[pos, name] : unresolved_labels) {
      if (labels.find(name) == labels.end()) {
        throw std::runtime_error("Undefined label: " + name);
      }
      
      uint64_t addr = labels[name];
      
      uint8_t* addrBytes = to_bin(addr);
      for (size_t i = 0; i < sizeof(uint64_t); i++) {
        binaryData[pos + i] = addrBytes[i];
      }

      delete[] addrBytes;
    }
    unresolved_labels.clear();
  }
};

uint8_t  db(uint8_t  what) { return what; }
uint16_t dw(uint16_t what) { return what; }
uint32_t dd(uint32_t what) { return what; }
uint64_t dq(uint64_t what) { return what; }

template <typename T>
std::vector<T> times(T value, int _times) {
  std::vector<T> vec{};
  
  for (int i = 0; i < _times; i++) vec.push_back(value);
  
  return vec;
}

std::vector<uint8_t> lea(uint8_t r1, uint64_t addr) {
  return BinarySerializer{ops::lea, r1, addr}.copy();
}

std::vector<uint8_t> store(uint8_t size, uint8_t r1, uint64_t org) {
  return BinarySerializer{ops::store, size, r1, org}.copy();
}

template <typename T>
std::vector<uint8_t> load(uint8_t to, uint8_t size, T what) {
  return BinarySerializer{ops::load, size, to, what}.copy();
}

std::vector<uint8_t> loadat(uint8_t to, uint64_t at) {
  return BinarySerializer{ops::loadat, to, at}.copy();
}

uint8_t nop() { return ops::nop; }
uint8_t ret() { return ops::ret; }
std::vector<uint8_t> mov(uint8_t a, uint8_t b) { return {ops::mov, a, b}; }
std::vector<uint8_t> add(uint8_t a, uint8_t b) { return {ops::add, a, b}; }
std::vector<uint8_t> sub(uint8_t a, uint8_t b) { return {ops::sub, a, b}; }
std::vector<uint8_t> mul(uint8_t a, uint8_t b) { return {ops::mul, a, b}; }
std::vector<uint8_t> wdiv(uint8_t a, uint8_t b){ return {ops::odiv, a, b}; }
std::vector<uint8_t> mod(uint8_t a, uint8_t b) { return {ops::mod, a, b}; }

std::vector<uint8_t> jmp(uint64_t where) { 
  return BinarySerializer{ops::jmp, (uint64_t)where}.copy();
}

std::vector<uint8_t> je(uint64_t where) { 
  return BinarySerializer{ops::je, where}.copy();
}

std::vector<uint8_t> jne(uint64_t where) { 
  return BinarySerializer{ops::jne, where}.copy();
}

std::vector<uint8_t> jl(uint64_t where) { 
  return BinarySerializer{ops::jl, where}.copy();
}

std::vector<uint8_t> jg(uint64_t where) { 
  return BinarySerializer{ops::jg, where}.copy();
}

std::vector<uint8_t> jle(uint64_t where) { 
  return BinarySerializer{ops::jle, where}.copy();
}

std::vector<uint8_t> jge(uint64_t where) { 
  return BinarySerializer{ops::jge, where}.copy();
}

std::vector<uint8_t> cmp(uint8_t a, uint8_t b) {
  return {ops::cmp, a, b};
}

std::vector<uint8_t> interrupt(uint8_t _code) {
  return {ops::xint, _code};
}

std::vector<uint8_t> movad(uint8_t to, uint8_t from) {
  return {ops::movad, to, from};
}

int main() {
  #define counter uint8_t(61)
  #define rv      uint8_t(50)
  // KEYBOARD Beg in dec:   419430400
  //              in hex: 0x19000000

  BinarySerializer buff {
    /* Read from STDIN, and write the line into STDOUT
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

    db(0xFF)*/


    
  };
 
  buff.resolveLabels();
  

  std::ofstream out("out.bin");
  out.write((const char*)buff.getBinaryData().data(), buff.getBinaryData().size());
  
  out.close();
  return 0;
}