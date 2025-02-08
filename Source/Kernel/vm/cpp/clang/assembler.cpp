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

std::string upper(const std::string &str) {
  std::string buff = "";
  for (const auto &c : str) {
    buff += std::toupper(c);
  }
  return buff;
}

bool is_int(const std::string &str) {
  for (const auto &c : str) {
    if (!isdigit(c)) return false;
  }

  return true;
}

bool is_number(const std::string &str) {
  for (const auto &c : str) {
    if (!isdigit(c) && c != '.') return false;
  }

  return true;
}

class assembler {
private:
  std::istream &input;
  std::ostream &output;

  std::unordered_map<std::string, uint8_t> table = {
    {"nop", NOP}, {"load", LOAD}, {"store", STORE}, 
    {"mov", MOV}, {"add", ADD}, {"sub", SUB}, {"mul", MUL}, 
    {"div", DIV}, {"mod", MOD}, 
    {"jmp", JMP}, {"je", JE}, {"jne", JNE}, {"jg", JG}, {"jl", JL}, 
    {"cmp", CMP}, {"call", CALL}, {"ret", RET},
    {"xor", XOR}, {"and", AND}, {"or", OR}, 
    {"syscall", SYSCALL}, {"halt", HALT},
  };

  void end(std::istringstream &iss) {
    if (!iss.eof()) {
      std::cerr << "Warn: Extra token (not compiled) after end of instruction. To compile them, create a new line." << std::endl;
    }
  }

  void write(std::vector<uint8_t> ops) {
    output.write(reinterpret_cast<const char*>(ops.data()), ops.size());
  }

  void compile_line(const std::string &line) {
    std::istringstream iss(line);
    std::string word;

    while (iss >> word) {
      if (word == "nop") {
        write({table["nop"]});
      } else if (word == "load") {
        
      } else {
        std::cerr << "Unknown symbol: " << word << ". \nLine: " << line << std::endl;
        return;
      }

      end(iss);
    }
  }

public:
  void compile() {
    std::string line;
    while (std::getline(input, line)) {

    }
  }
};