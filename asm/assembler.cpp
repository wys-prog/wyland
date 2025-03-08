#include <iostream>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>

#include "asm/settable.hpp"

enum TokenType {
  INSTRUCTION,
  DATA,
  LABEL,
  STRING, 
};

struct Token {
  TokenType type;
  std::string value;
};

class BinarySerializer {
private:
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

};

class Assembler {
private:
  std::istream &input;
  std::ostream &output;
  std::unordered_map<std::string, uint64_t> labels;
  uint64_t position = 0;

  void write(const std::string &str) {
    output.write(str.c_str(), str.size());
  }

  void parseInstruction(Token token) {
    try {
      if (token.value.size() >= 2 && (token.value.substr(0, 2) == "0x" || token.value.substr(0, 2) == "0X" )) {
        BinarySerializer serializer{std::stoull(token.value, 0, 16)};
        output.write((char*)serializer.copy().data(), serializer.copy().size());
      } else if (token.value.size()) {
        
      }

    } catch (const std::invalid_argument &e) {
      throw std::runtime_error(e.what());
    } catch (const std::out_of_range &e) {
      throw std::runtime_error(e.what());
    } catch (const std::exception &e) {
      throw std::runtime_error(e.what());
    }
  }

  void defineLabel(const std::string &name) {
    if (labels.count(name)) {
      throw std::runtime_error("Redefinition of label: " + name);
    }
    labels[name] = position;
  }

  uint64_t getLabelAddress(const std::string &name) {
    if (!labels.count(name)) {
      throw std::runtime_error("Undefined label: " + name);
    }
    return labels[name];
  }

  void compileLine(std::vector<Token> tokens) {
    for (size_t i = 0; i < tokens.size(); i++) {
      switch (tokens[i].type) {
        case DATA:        break;
        case INSTRUCTION: break;
        case LABEL:       break;
        default: throw std::runtime_error("Invalid token type.");
      }
    }
  }

public:
  Assembler(std::istream &in, std::ostream &out) : input(in), output(out) {}

  void compile() {
    std::string line;
    
    while (std::getline(input, line)) {
      std::vector<Token> tokens;
      std::istringstream iss(line);
      std::string word;
      char c;

      while (iss >> std::noskipws >> c) {
        word.clear();
        
        if (c == '\\' && iss.peek() == '\\') {
          iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          break;
        }
        
        if (c == '\'') { 
          iss >> std::noskipws >> c;
          while (c != '\'') {
            if (c == '\\') {
              iss >> c;
              switch (c) {
                case 'n': word.push_back('\n'); break;
                case 't': word.push_back('\t'); break;
                case 'r': word.push_back('\r'); break;
                case 'b': word.push_back('\b'); break;
                case 'f': word.push_back('\f'); break;
                case 'a': word.push_back('\a'); break;
                case 'v': word.push_back('\v'); break;
                case '\\': word.push_back('\\'); break;
                case '\'': word.push_back('\''); break;
                case '\"': word.push_back('\"'); break;
                case '\?': word.push_back('\?'); break;
                default: word.push_back(c); break;
              }
            } else {
              word.push_back(c);
            }
            iss >> c;
          }
          tokens.push_back({STRING, word});
        } else if (c == '%') {
          iss >> word;
          tokens.push_back({INSTRUCTION, word});
        } else if (c == '@') {
          iss >> word;
          tokens.push_back({LABEL, word});
        }
      }

      // Compile the line.
      compileLine(tokens);
      tokens.clear();
    }
  }
};
