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
        } else if (false) {
          
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