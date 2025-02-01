#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <functional>
#include <unordered_map>

std::unordered_map<std::string, uint8_t> instructions = {
    {"NOP",  0x00},
    {"LOAD", 0x01},
    {"MOV",  0x02},
    {"ADD",  0x03},
    {"SUB",  0x04},
    {"MUL",  0x05},
    {"DIV",  0x06},
    {"MOD",  0x07},
    {"JMP",  0x08},
    {"JE",   0x09},
    {"JNE",  0x0A},
    {"JG",   0x0B},
    {"JL",   0x0C},
    {"CMP",  0x0D},
    {"CALL", 0x0E},
    {"RET",  0x0F},
    {"XOR",  0x10},
    {"OR",   0x11},
    {"AND",  0x12},
    {"HALT", 0xFF}, 

    {"R0",  0x00},
    {"R1",  0x01},
    {"R2",  0x02},
    {"R3",  0x03},
    {"R4",  0x04},
    {"R5",  0x05},
    {"R6",  0x06},
    {"R7",  0x07},
    {"R8",  0x08},
    {"R9",  0x09},
    {"R10", 0x0A},
    {"R11", 0x0B},
    {"R12", 0x0C},
    {"R13", 0x0D},
    {"R14", 0x0E},
    {"R15", 0x0F},
    {"R16", 0x10},
    {"R17", 0x11},
    {"R18", 0x12},
    {"R19", 0x13},
    {"R20", 0x14},
    {"R21", 0x15},
    {"R22", 0x16},
    {"R23", 0x17},
    {"R24", 0x18},
    {"R25", 0x19},
    {"R26", 0x1A},
    {"R27", 0x1B},
    {"R28", 0x1C},
    {"R29", 0x1D},
    {"R30", 0x1E},
    {"R31", 0x1F}
};

std::string str2up(const std::string &str) {
  std::string buff = "";
  for (const auto &c : str) buff += toupper(c);
  return buff;
}

class multios {
private:
  std::vector<std::ofstream> outputs;

public:
  multios() = default;

  void append(const std::string &path) {
    std::ofstream file(path);

    if (!file) throw std::runtime_error("No such file: " + path);

    outputs.push_back(file);
  }

  multios(std::vector<std::string> files) {
    for (const auto &file : files) append(file);
  }

  template <typename T>
  multios &operator<<(const T &__T) {
    for (auto &output : outputs) 
      output << __T;
    return *this;
  }

  void close() {
    for (auto &output : outputs) 
      output.close();
  }

  void open(const std::string &path) { append(path); }

  void open(std::vector<std::string> files) {
    for (const auto &file : files) append(file);
  }
};

class strong_compiler {
private:
  

  void times(int _count) {}

};