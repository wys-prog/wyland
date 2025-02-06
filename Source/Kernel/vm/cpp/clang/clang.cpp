#include <string>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <functional>
#include <unordered_map>

#include "clang.h"

std::string lower(const std::string &str) {
  std::string buff = "";
  for (const auto &c : str) {
    buff += std::tolower(c);
  }
  return buff;
}

class compiler {
private:
  std::unordered_map<std::string, uint8_t> table = {
    {"nop", NOP}, {"load", LOAD}, {"store", STORE}, 
    {"mov", MOV}, {"add", ADD}, {"sub", SUB}, {"mul", MUL}, 
    {"div", DIV}, {"mod", MOD}, 
    {"jmp", JMP}, {"je", JE}, {"jne", JNE}, {"jg", JG}, {"jl", JL}, 
    {"cmp", CMP}, {"call", CALL}, {"ret", RET},
    {"xor", XOR}, {"and", AND}, {"or", OR}, 
    {"syscall", SYSCALL}, {"halt", HALT},
  };

    std::unordered_map<std::string, uint8_t> registers = {
    {"r0", 0}, {"r1", 1}, {"r2", 2}, {"r3", 3},
    {"r4", 4}, {"r5", 5}, {"r6", 6}, {"r7", 7},
    {"r8", 8}, {"r9", 9}, {"r10", 10}, {"r11", 11},
    {"r12", 12}, {"r13", 13}, {"r14", 14}, {"r15", 15},
    {"r16", 16}, {"r17", 17}, {"r18", 18}, {"r19", 19},
    {"r20", 20}, {"r21", 21}, {"r22", 22}, {"r23", 23},
    {"r24", 24}, {"r25", 25}, {"r26", 26}, {"r27", 27},
    {"r28", 28}, {"r29", 29}, {"r30", 30}, {"r31", 31},
  };

  std::istream &input;
  std::ostream &output;

  void translate() {
    std::string line;
    while (std::getline(input, line)) {
      std::istringstream iss(line);
      std::string word;

      while (iss >> word) {
        if (table.find(lower(word)) == table.end()) {
          char buff[1] = {table[lower(word)]};
          output.write(buff, 1);
        } else if (word.starts_with(';')) {
          std::getline(iss, line); // Skip the line.
        } else if (word.starts_with('$')) {
          if (registers.find(lower(word.substr(1))) != registers.end()) {
            char buff[1] = {registers[lower(word.substr(1))]};
            output.write(buff, 1);
          }
        } else if (lower(word) == "@int") {
          int64_t i;
          iss >> i;
          if (iss.fail()) throw std::invalid_argument("Not a number");
          output.write(reinterpret_cast<const char*>(i), sizeof(i));
        } else if (lower(word) == "@uint") {
          uint64_t i;
          iss >> i;
          if (iss.fail()) throw std::invalid_argument("Not a number");
          output.write(reinterpret_cast<const char*>(i), sizeof(i));
        } else { // Unknown symbol. Represents data.
          char *buff = new char[word.size()];
          output.write(buff, word.size());
          delete[] buff; 
        }
        
      }
    }
  }

public:
};