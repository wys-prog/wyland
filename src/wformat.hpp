#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cstdint>

namespace wylma {
  namespace wyland {

    template <typename TyVec>
    std::string format(const std::initializer_list<TyVec> &v, char del = ' ') {
      std::string my_format = "";
      for (const auto &e:v) my_format += std::to_string(e) + del;
      my_format.pop_back();
      return my_format;
    }

    std::string format(const std::initializer_list<uint8_t> &v, char del = ' ') {
      std::ostringstream oss;

      oss << std::hex << std::uppercase;

      for (const auto&e:v) oss << (int)e << del;
      
      auto my_fmt = oss.str();
      
      my_fmt.pop_back();

      return my_fmt;
    }

    std::string format(const std::string &raw_string) {
      std::string result;
      for (char c : raw_string) {
        if (static_cast<unsigned char>(c) < 128) {
          result += c;
        }
      }
      return result;
    }

    template <typename T>
    std::string format(const T *array, size_t len) {
      std::string result;
      for (size_t i = 0; i < len; i++) {
        result += std::to_string(array[i]);
      }

      return result;
    }

    std::string format(const uint8_t *array, size_t len) {
      std::string result;
      for (size_t i = 0; i < len; i++) {
        result += (char)(array[i]); 
        result += ", ";
      }

      result.pop_back();
      result.pop_back();

      return result;
    }

    std::istringstream to_string(const std::u8string &bytestring) {
      return std::istringstream((char*)bytestring.c_str());
    }
  }
}