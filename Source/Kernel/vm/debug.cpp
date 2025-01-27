#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <unordered_map>

void write_binary_file(const std::string &filename, const std::vector<uint8_t> &instructions, bool append = false) {
  std::ofstream file(filename, std::ios::binary);

  if (append) file.open(filename, std::ios::binary | std::ios::app);
  else file.open(filename, std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Error: Unable to open file for writing." << std::endl;
    return;
  }

  file.write(reinterpret_cast<const char *>(instructions.data()), instructions.size()); 
  file.close();

  std::cout << "File " << filename << " successfully written with "
            << instructions.size() << " bytes." << std::endl;
}

std::unordered_map<std::string, uint8_t> ops = {
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

std::string to_upper(const std::string &input) {
  std::string result = input;
  for (char& c : result) {
    c = std::toupper(c);
  }
  return result;
}

std::string to_lower(const std::string &input) {
  std::string result = input;
  for (char& c : result) {
    c = std::tolower(c);
  }
  return result;
}

void compile(std::istream &is, const std::string &filename) {
  std::string line;
  std::vector<uint8_t> ins;

  while (std::getline(is, line)) {
    std::istringstream iss(line);
    std::string word;

    while (iss >> word) {
      if (ops.find(to_upper(word)) != ops.end()) {
        ins.push_back(ops[to_upper(word)]);
      } else {
        std::vector<uint8_t> vec(word.size());
        std::copy(word.begin(), word.end(), vec.begin());
        ins.insert(ins.end(), word.begin(), word.end());
      }

      if (ins.size() >= 1024) {
        write_binary_file(filename, ins, true);
        ins.clear();
      }
    }
  }

  write_binary_file(filename, ins, true);
}

int main() {
  std::string filename;
  std::cout << "Enter output filename: ";
  std::cin >> filename;

  // Instructions to test the VM
  std::vector<uint8_t> instructions = {
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, // LOAD R0, 10
      0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, // LOAD R1, 20
      0x03, 0x00, 0x01,                                           // ADD R0, R1
      0x0D, 0x00, 0x01,                                           // CMP R0, R1
      0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,       // JG 0x20 
      0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // LOAD R2, 1
      0x07, 0x00, 0x02,                                           // MOD R0, R2
      0xFF                                                        // HALT
  };

  // Write the binary file
  write_binary_file(filename, instructions);

  return 0;
}
