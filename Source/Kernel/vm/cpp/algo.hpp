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
} // namespace wyland
