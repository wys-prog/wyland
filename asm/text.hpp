#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "binary.hpp"

namespace Wys {

  template <typename TyCharEncoding>
  class BasicString {
  private:
    std::basic_string<TyCharEncoding> data;

  public:
    BasicString() = default;

    BasicString(const TyCharEncoding* cstr) : data(cstr) {}

    BasicString(const std::basic_string<TyCharEncoding>& str) : data(str) {}

    BasicString(const BasicString<TyCharEncoding>& other) : data(other.data) {}

    BasicString(BasicString<TyCharEncoding>&& other) noexcept : data(std::move(other.data)) {}

    BasicString<TyCharEncoding>& operator=(const BasicString<TyCharEncoding>& other) {
      if (this != &other) {
        data = other.data;
      }
      return *this;
    }

    BasicString<TyCharEncoding>& operator=(BasicString<TyCharEncoding>&& other) noexcept {
      if (this != &other) {
        data = std::move(other.data);
      }
      return *this;
    }

    BasicString<TyCharEncoding>& operator=(const TyCharEncoding* cstr) {
      data = cstr;
      return *this;
    }

    BasicString<TyCharEncoding>& operator=(const std::basic_string<TyCharEncoding>& str) {
      data = str;
      return *this;
    }

    size_t IndexOf(const BasicString<TyCharEncoding> &str) {
      return data.find(str.data);
    }
    
    size_t IndexOf(const std::basic_string<TyCharEncoding> &str) {
      return data.find(str);
    }
    
    size_t IndexOf(const TyCharEncoding &c) {
      return data.find(c);
    }
    
    size_t CountOf(const BasicString<TyCharEncoding> &str) {
      size_t count = 0;
      size_t pos = 0;
      while ((pos = data.find(str.data, pos)) != std::string::npos) {
        ++count;
        pos += str.Size();
      }
      return count;
    }
    
    size_t CountOf(const std::basic_string<TyCharEncoding> &str) {
      size_t count = 0;
      size_t pos = 0;
      while ((pos = data.find(str, pos)) != std::string::npos) {
        ++count;
        pos += str.size();
      }
      return count;
    }
    
    size_t CountOf(const TyCharEncoding &c) {
      return std::count(data.begin(), data.end(), c);
    }

    size_t CountOfThem(const BasicString<TyCharEncoding> &str) {
      size_t count = 0;
      for (size_t i = 0; i < str.Size(); i++) count += CountOf(str.Base[i]);
      return count;
    }

    TyCharEncoding* Raw() { return &data[0]; }
    const TyCharEncoding* Raw() const { return data.c_str(); }

    size_t Size() const { return data.size(); }
    size_t Capacity() const { return data.capacity(); }

    void Append(const TyCharEncoding& c) {
      data.push_back(c);
    }

    void Append(const BasicString<TyCharEncoding>& str) {
      data.append(str.data);
    }

    std::basic_string<TyCharEncoding> Base() const { return data; }

    BasicString<TyCharEncoding> Substr(size_t start, size_t length) const {
      if (start >= data.size()) return BasicString<TyCharEncoding>();
      length = std::min(length, data.size() - start);
      return BasicString<TyCharEncoding>(data.substr(start, length));
    }

    std::vector<BasicString<TyCharEncoding>> Split(const BasicString<TyCharEncoding>& pattern) const {
      std::vector<BasicString<TyCharEncoding>> result;
      size_t start = 0;
      size_t end = 0;

      while ((end = data.find(pattern.data, start)) != std::string::npos) {
        result.push_back(Substr(start, end - start));
        start = end + pattern.Size();
      }

      result.push_back(Substr(start, data.size() - start));
      return result;
    }

    std::vector<BasicString<TyCharEncoding>> Split(const std::basic_string<TyCharEncoding>& pattern) const {
      return Split(BasicString<TyCharEncoding>(pattern));
    }

    std::vector<BasicString<TyCharEncoding>> SplitMultiple(const std::vector<BasicString<TyCharEncoding>>& patterns) const {
      std::vector<BasicString<TyCharEncoding>> result;
      size_t start = 0;
      size_t end = std::string::npos;

      while (start < data.size()) {
        end = std::string::npos;
        size_t pattern_length = 0;

        for (const auto& pattern : patterns) {
          size_t pos = data.find(pattern.data, start);
          if (pos != std::string::npos && (end == std::string::npos || pos < end)) {
            end = pos;
            pattern_length = pattern.Size();
          }
        }

        if (end != std::string::npos) {
          result.push_back(Substr(start, end - start));
          start = end + pattern_length;
        } else {
          result.push_back(Substr(start, data.size() - start));
          break;
        }
      }

      return result;
    }

    std::vector<BasicString<TyCharEncoding>> SplitMultiple(const std::vector<std::basic_string<TyCharEncoding>>& patterns) const {
      std::vector<BasicString<TyCharEncoding>> basic_patterns;
      for (const auto& pattern : patterns) {
        basic_patterns.push_back(BasicString<TyCharEncoding>(pattern));
      }
      return SplitMultiple(basic_patterns);
    }

    void ReplaceBy(const BasicString<TyCharEncoding>& pattern, const BasicString<TyCharEncoding>& with) {
      size_t pos = 0;
      while ((pos = data.find(pattern.data, pos)) != std::string::npos) {
        data.replace(pos, pattern.Size(), with.data);
        pos += with.Size();
      }
    }

    void ReplaceBy(const std::basic_string<TyCharEncoding>& pattern, const std::basic_string<TyCharEncoding>& with) {
      ReplaceBy(BasicString<TyCharEncoding>(pattern), BasicString<TyCharEncoding>(with));
    }

    std::vector<uint8_t> ToByteArray() const {
      std::vector<uint8_t> buff;
      for (size_t i = 0; i < data.size(); i++) {
        auto serialized = (Serialize(this->data[i]));
        for (const auto&e:serialized) buff.push_back(e);
      }
      return buff;
    }

    BasicString<TyCharEncoding> Copy() const {
      BasicString<TyCharEncoding> buff;
      for (const auto&e:data) {
        buff.Append(e);
      }
      return buff;
    }

    void Reverse() {
      std::reverse(data.begin(), data.end());
    }
  
    bool StartsWith(const BasicString<TyCharEncoding>& prefix) const {
      return data.compare(0, prefix.Size(), prefix.data) == 0;
    }
  
    bool EndsWith(const BasicString<TyCharEncoding>& suffix) const {
      return data.size() >= suffix.Size() &&
            data.compare(data.size() - suffix.Size(), suffix.Size(), suffix.data) == 0;
    }
  
    void Trim(const BasicString<TyCharEncoding> &with = " ") {
      auto start = data.find_first_not_of(with.Base());
      auto end = data.find_last_not_of(with.Base());
      data = (start == std::string::npos) ? std::basic_string<TyCharEncoding>() : data.substr(start, end - start + 1);  
    }

    void FromBytes(const std::vector<uint8_t> &bytes) {
      data.clear();
      size_t charSize = sizeof(TyCharEncoding);
  
      if (bytes.size() % charSize != 0) {
        throw std::runtime_error("Invalid byte array size for this character type");
      }
  
      for (size_t i = 0; i < bytes.size(); i += charSize) {
        TyCharEncoding ch = 0;
        for (size_t j = 0; j < charSize; ++j) {
          ch |= static_cast<TyCharEncoding>(bytes[i + j]) << (8 * j);
        }
        data.push_back(ch);
      }
    }

    std::hash<std::basic_string<TyCharEncoding>> ToHash() const {
      std::hash<std::basic_string<TyCharEncoding>> hasher;
      return (hasher(data));
    }

    long long ToInt() const {
      std::string string(data.begin(), data.end());
      if (string.starts_with("0x") || string.starts_with("0X")) return std::stoll(string, 0, 16);
      else if (string.starts_with("0o") || string.starts_with("0O")) return std::stoll(string, 0, 8);
      return std::stoll(string);
    }

    unsigned long long ToUint() const {
      std::string string(data.begin(), data.end());
      if (string.starts_with("0x") || string.starts_with("0X")) return std::stoull(string, 0, 16);
      else if (string.starts_with("0o") || string.starts_with("0O")) return std::stoull(string, 0, 8);
      return std::stoull(string);
    }

    std::vector<BasicString<TyCharEncoding>> ExtractArguments() const {
      std::vector<BasicString<TyCharEncoding>> result;
      std::basic_string<TyCharEncoding> current;
      bool in_double_quotes = false;
      bool in_single_quotes = false;

      for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] == '"') {
          if (in_single_quotes) {
            current.push_back(data[i]);
          } else {
            in_double_quotes = !in_double_quotes;
            if (!in_double_quotes && !current.empty()) {
              result.push_back(current);
              current.clear();
            }
          }
        } else if (data[i] == '\'') {
          if (in_double_quotes) {
            current.push_back(data[i]);
          } else {
            in_single_quotes = !in_single_quotes;
            if (!in_single_quotes && !current.empty()) {
              result.push_back(current);
              current.clear();
            }
          }
        } else if (std::isspace(data[i]) && !in_double_quotes && !in_single_quotes) {
          if (!current.empty()) {
            result.push_back(current);
            current.clear();
          }
        } else {
          current.push_back(data[i]);
        }
      }

      if (!current.empty()) {
        result.push_back(current);
      }

      return result;
    }

    bool IsNumber() const {
      return !data.empty() && std::all_of(data.begin(), data.end(), ::isdigit);
    }

    bool IsNegatifNumber() const {
      return !data.empty() && data[0] == '-' && std::all_of(data.begin() + 1, data.end(), ::isdigit);
    }

    bool operator==(const BasicString<TyCharEncoding>& other) const {
      return data == other.data;
    }

    bool operator!=(const BasicString<TyCharEncoding>& other) const {
      return !(*this == other);
    }

    bool operator<(const BasicString<TyCharEncoding>& other) const {
      return data < other.data;
    }

    bool operator>(const BasicString<TyCharEncoding>& other) const {
      return other < *this;
    }

    bool operator<=(const BasicString<TyCharEncoding>& other) const {
      return !(other < *this);
    }

    bool operator>=(const BasicString<TyCharEncoding>& other) const {
      return !(*this < other);
    }

    friend std::ostream& operator<<(std::ostream& os, const BasicString<TyCharEncoding>& str) {
      return os << str.data;
    }

    friend std::istream& operator>>(std::istream& is, BasicString<TyCharEncoding>& str) {
      return is >> str.data;
    }

    TyCharEncoding &operator[](size_t index) {
      return data[index];
    }
  };

  template <typename TyCharEncoding, typename TyCharReal>
  class String : public BasicString<TyCharEncoding> {
  public:
    String() : BasicString<TyCharEncoding>() {}

    String(const BasicString<TyCharEncoding> &base) : BasicString<TyCharEncoding>(base) {}

    String(const TyCharReal* cstr)
      : BasicString<TyCharEncoding>(reinterpret_cast<const TyCharEncoding*>(cstr)) {}

    String(const std::basic_string<TyCharReal>& string)
      : BasicString<TyCharEncoding>(reinterpret_cast<const TyCharEncoding*>(string.c_str())) {}

    String(const String<TyCharEncoding, TyCharReal>& other)
      : BasicString<TyCharEncoding>(other) {}

    String<TyCharEncoding, TyCharReal>& operator=(const TyCharReal* ptr) {
      BasicString<TyCharEncoding>::operator=(reinterpret_cast<const TyCharEncoding*>(ptr));
      return *this;
    }

    String<TyCharEncoding, TyCharReal>& operator=(const std::basic_string<TyCharReal>& str) {
      BasicString<TyCharEncoding>::operator=(reinterpret_cast<const TyCharEncoding*>(str.c_str()));
      return *this;
    }

    String<TyCharEncoding, TyCharReal>& operator=(const String<TyCharEncoding, TyCharReal>& other) {
      BasicString<TyCharEncoding>::operator=(other);
      return *this;
    }

    String<TyCharEncoding, TyCharReal> operator+(const String<TyCharEncoding, TyCharReal>& other) const {
      String<TyCharEncoding, TyCharReal> result(*this);
      result.Append(other);
      return result;
    }

    String<TyCharEncoding, TyCharReal> operator+(const TyCharReal* ptr) const {
      String<TyCharEncoding, TyCharReal> result(*this);
      result.Append(ptr);
      return result;
    }

    String<TyCharEncoding, TyCharReal>& operator+=(const String<TyCharEncoding, TyCharReal>& other) {
      BasicString<TyCharEncoding>::Append(other);
      return *this;
    }

    String<TyCharEncoding, TyCharReal>& operator+=(const TyCharReal* ptr) {
      BasicString<TyCharEncoding>::Append(reinterpret_cast<const TyCharEncoding*>(ptr));
      return *this;
    }

    bool operator==(const String<TyCharEncoding, TyCharReal>& other) const {
      return BasicString<TyCharEncoding>::operator==(other);
    }

    bool operator!=(const String<TyCharEncoding, TyCharReal>& other) const {
      return !(*this == other);
    }

    bool operator<(const String<TyCharEncoding, TyCharReal>& other) const {
      return BasicString<TyCharEncoding>::operator<(other);
    }

    bool operator>(const String<TyCharEncoding, TyCharReal>& other) const {
      return other < *this;
    }

    bool operator<=(const String<TyCharEncoding, TyCharReal>& other) const {
      return !(other < *this);
    }

    bool operator>=(const String<TyCharEncoding, TyCharReal>& other) const {
      return !(*this < other);
    }

    friend std::ostream& operator<<(std::ostream& os, const String<TyCharEncoding, TyCharReal>& str) {
      return os << reinterpret_cast<const TyCharReal*>(str.Raw());
    }

    friend std::istream& operator>>(std::istream& is, String<TyCharEncoding, TyCharReal>& str) {
      std::basic_string<TyCharReal> input;
      is >> input;
      str = input;
      return is;
    }

    std::basic_string<TyCharReal> ToString() const {
      return std::basic_string<TyCharReal>(this->Raw());
    }

    BasicString<TyCharEncoding> ToBasicString() const {
      return this->Copy();
    }
  };

  using StringUTF8 = String<uint8_t, char>;
  using StringUTF16 = String<uint16_t, char16_t>;
  using StringUTF32 = String<uint32_t, char32_t>;
  using StringASCII = String<uint8_t, char>;
}

inline Wys::StringUTF8 operator""_utf8(const char *str, size_t) {
  return Wys::StringUTF8(str);
}

inline Wys::StringUTF16 operator""_utf16(const char16_t *str, size_t) {
  return Wys::StringUTF16(str);
}

inline Wys::StringUTF32 operator""_utf32(const char32_t *str, size_t) {
  return Wys::StringUTF32(str);
}