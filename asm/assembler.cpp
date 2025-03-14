#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <type_traits>

template <typename T>
std::vector<uint8_t> to_big_endian(T value) {
  static_assert(std::is_integral<T>::value, "T must be an integral type");
  std::vector<uint8_t> bytes(sizeof(T));
  for (size_t i = 0; i < sizeof(T); ++i) {
    bytes[sizeof(T) - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
  }
  return bytes;
}

class Label {
private:
  std::string Name;
  uint64_t    Address;
  std::vector<uint8_t> Bytes;
  std::vector<Label*>  Childs;
  std::vector<char> Schem;

public:
  Label() = default;

  Label(const std::string name, uint64_t address, const std::vector<uint8_t> &bytes, 
        const std::vector<Label*> &childs, const std::vector<char> &schem) 
    : Name(name), Address(address), Bytes(bytes), Childs(childs), Schem(schem)
    {} 
  
  uint64_t Get() const { return Address; }

  std::vector<uint8_t> Compile() const {
    std::vector<uint8_t> buff{};
    size_t bytesPos = 0;
    size_t labelPos = 0;

    for (const auto&c:Schem) {
      if (c == 'l') {
        auto tmp = Childs[labelPos]->Compile();
        for (const auto&e:tmp) buff.push_back(e);
        labelPos++;
      } else {
        buff.push_back(Bytes[bytesPos++]);
      }
    }

    return buff;
  }
};

class Constant {
private:
  std::string Patern;
  std::vector<std::string> Arguments;

public:
  std::string Expand(std::vector<std::string> arguments) const {
    if (arguments.size() != Arguments.size()) {
      throw std::invalid_argument(arguments.size() > Arguments.size() ? "Too much arguments for Constant." : "Too few arguments for Constant.");
    }

    std::unordered_map<std::string, std::string> map{};
    for (size_t i = 0; i < arguments.size(); i++) 
      map[Arguments[i]] = arguments[i];
    
    // Replace arguments in pattern.

    std::string buff = "";

    for (size_t i = 0; i < Patern.size(); i++) {
      // It's an argument.
      if (Patern[i] == '$') {
        i++;
        std::string argument = "", whiteSpaces;
        
        // Get the word.
        while (std::iswspace(Patern[i])) whiteSpaces += Patern[i++];
        while (!std::iswspace(Patern[i])) argument += Patern[i++];

        if (map.find(argument) == map.end()) buff += '$' + whiteSpaces;
        else buff += map[argument] + argument;
      } else {
        buff += Patern[i];
      }
    }

    return buff;
  }

  Constant() = default;
  Constant(const std::string &patern, const std::vector<std::string> arguments) 
    : Patern(patern), Arguments(arguments)
  {}
};

class Assembler {
private:
  std::istream &Source;  
  std::ostream &Output;

  std::unordered_map<std::string, Label> Labels;
  std::unordered_map<std::string, Constant> Constants;

public:

  void Compile() {
    
  }
};