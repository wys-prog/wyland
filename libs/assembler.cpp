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

#include "binary.hpp"
#include "text.hpp"

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

  const std::vector<Wys::BasicString<Wys::UTF8>> instructions {
    "%db "_utf8, "%dw "_utf8, "%dd "_utf8, "%dq "_utf8, 
    /*"%load "_utf8, "%loadat "_utf8, "%sotre "_utf8, "%lea "_utf8, 
    "%mov "_utf8, "%add "_utf8, "%sub "_utf8, "%mul "_utf8, "%div "_utf8, "%mod "_utf8, "%movad "_utf8,
    "%jmp "_utf8, "%je "_utf8, "%jne "_utf8, "%jg "_utf8, "%jl "_utf8, "%jge "_utf8, "%jle "_utf8, 
    "%int "_utf8, 
    "%cmp "_utf8, 
    "%ret "_utf8, 
    "%nop "_utf8,*/
    "%times "_utf8,
    "%.utf8"_utf8, 
    "%.utf16"_utf8, 
    "%.utf32"_utf8, 
    "%.ascii"_utf8,
  };

  void db(const Wys::StringUTF8 &String) {
    auto args = String.ExtractArguments();
    for (const auto&arg:args) {
      if (!arg.IsNumber()) {
        Output.write((char*)arg.Base().c_str(), arg.Base().size());
      } else if (arg.IsNegatifNumber()) {
        char buffer[1]{(int8_t)arg.ToInt()};
        Output.write(buffer, sizeof(buffer));
      } else {
        char buffer[1]{(uint8_t)arg.ToUint()};
        Output.write(buffer, sizeof(buffer));
      }
    }
  }

  void dw(const Wys::StringUTF8 &String) {
    auto args = String.ExtractArguments();
    for (const auto&arg:args) {
      if (!arg.IsNumber()) {
        auto encoded = arg.EncodeOn<uint16_t>();
        for (const auto&e:encoded) {
          auto bytes = Wys::Serialize(e);
          Output.write((char*)bytes.data(), bytes.size());
        }
      } else if (arg.IsNegatifNumber()) {
        auto value = (int16_t)arg.ToInt();
        auto bytes = Wys::Serialize(value);
        Output.write((char*)bytes.data(), bytes.size());
      } else {
        auto value = (uint16_t)arg.ToUint();
        auto bytes = Wys::Serialize(value);
        Output.write((char*)bytes.data(), bytes.size());
      }
    }
  }

  void dd(const Wys::StringUTF8 &String) {
    auto args = String.ExtractArguments();
    for (const auto&arg:args) {
      if (!arg.IsNumber()) {
        auto encoded = arg.EncodeOn<uint32_t>();
        for (const auto&e:encoded) {
          auto bytes = Wys::Serialize(e);
          Output.write((char*)bytes.data(), bytes.size());
        }
      } else if (arg.IsNegatifNumber()) {
        auto value = (int32_t)arg.ToInt();
        auto bytes = Wys::Serialize(value);
        Output.write((char*)bytes.data(), bytes.size());
      } else {
        auto value = (uint32_t)arg.ToUint();
        auto bytes = Wys::Serialize(value);
        Output.write((char*)bytes.data(), bytes.size());
      }
    }
  }

  void dq(const Wys::StringUTF8 &String) {
    auto args = String.ExtractArguments();
    for (auto&arg:args) {
      arg.Trim();
      if (!arg.IsNumber()) {
        auto encoded = arg.EncodeOn<uint64_t>();
        for (const auto&e:encoded) {
          auto bytes = Wys::Serialize(e);
          Output.write((char*)bytes.data(), bytes.size());
        }
      } else if (arg.IsNegatifNumber()) {
        auto value = (int64_t)arg.ToInt();
        auto bytes = Wys::Serialize(value);
        Output.write((char*)bytes.data(), bytes.size());
      } else {
        auto value = (uint64_t)arg.ToUint();
        auto bytes = Wys::Serialize(value);
        Output.write((char*)bytes.data(), bytes.size());
      }
    }
  }

public:

  void Compile() {
    std::string buff;
    while (std::getline(Source, buff)) {
      Wys::StringUTF8 line(buff);
      auto tokens = line.SplitMultiple(instructions);
      for (auto&token:tokens) {
        token.Trim();
        if (token.StartsWith("%db "_utf8)) db(token.Substr(4, token.Size()-4));
        else if (token.StartsWith("%dw "_utf8)) dw(token.Substr(4, token.Size()-4));
        else if (token.StartsWith("%dd "_utf8)) dd(token.Substr(4, token.Size()-4));
        else if (token.StartsWith("%dq "_utf8)) dq(token.Substr(4, token.Size()-4));
        else if (token.StartsWith("%times "_utf8)) {
          
        }
      }
    }
  }
};
