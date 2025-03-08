#include <iostream>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdint>

#include "asm/settable.hpp"

class Assembler {
private:
  std::istream &input;
  std::ostream &output;

  std::unordered_map<std::string, uint64_t> labels;
  std::unordered_map<std::string, std::string> macros;

  uint64_t pos = 0x0000000000000000;
  
  void define(const std::string &name) {
    if (labels.find(name) != labels.end()) {
      std::cerr << "Redefinition of label: " << name << std::endl;
      throw std::runtime_error("Redefinition error");
    }

    labels[name] = pos;
  }

  uint64_t reference(const std::string &name) {
    if (labels.find(name) == labels.end()) {

    }

    return labels[name];
  }



public:

  void Expand(); 

  void Compile() {
    
  }
};