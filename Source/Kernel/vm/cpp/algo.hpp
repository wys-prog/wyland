#pragma once

#include <string>
#include <vector>

namespace wyland {
  void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
      return !std::isspace(ch);
    }));
  }
  
  void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
      return !std::isspace(ch);
    }).base(), s.end());
  }
  
  void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
  }
  
  std::string trimRT(const std::string &s) {
    std::string tmp = s;
    trim(tmp);
    return tmp;
  }

  void trim_vector(std::vector<std::string> &vector) {
    for (size_t i = 0; i < vector.size(); i++) {
      trim(vector[i]);
    }
  }

  std::vector<std::string> select_only(const std::string &what, 
    const std::vector<std::string> &on) {
    std::vector<std::string> buff;

    for (const auto &elem : on) {
      if (trimRT(elem).starts_with(what)) { buff.push_back(elem); }
    }

    return buff;
  }

  template <typename T> 
  std::string to_string(const std::vector<T> &vecT) {
    std::string buff = "[";
    for (const auto &elem : vecT) {
      buff += std::to_string(elem) + ", ";
    }
    buff.pop_back();
    buff.pop_back();
    buff += "]";
    return buff;
  }

  std::string to_string(const std::vector<std::string> &vecT) {
    std::string buff = "";
    for (const auto &elem : vecT) {
      buff += elem;
    }
    return buff;
  }

  template <typename T> 
  T extract_first(std::vector<T> &vecT) {
    if (vecT.empty()) { return T(); }
    T buff = vecT[0];
    vecT.erase(vecT.begin());
    return buff;
  }
} // namespace wyland
