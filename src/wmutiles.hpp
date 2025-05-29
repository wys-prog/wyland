#pragma once

#ifdef _WIN32
#include <windows.h>
#endif 

#include "wmmbase.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/map.hpp>

WYLAND_BEGIN



  // std::vector
  template<typename T>
  void destroy(std::vector<T>& vec) {
    std::vector<T>().swap(vec);
  }

  // std::string
  inline void destroy(std::string& str) {
    std::string().swap(str);
  }

  // std::unordered_map
  template<typename K, typename V>
  void destroy(std::unordered_map<K, V>& map) {
    std::unordered_map<K, V>().swap(map);
  }

  // std::unordered_set
  template<typename T>
  void destroy(std::unordered_set<T>& set) {
    std::unordered_set<T>().swap(set);
  }

  // boost::unordered_map
  template<typename K, typename V>
  void destroy(boost::unordered_map<K, V>& map) {
    boost::unordered_map<K, V>().swap(map);
  }

  // boost::unordered_set
  template<typename T>
  void destroy(boost::unordered_set<T>& set) {
    boost::unordered_set<T>().swap(set);
  }

  // boost::container::flat_map
  template<typename K, typename V>
  void destroy(boost::container::flat_map<K, V>& map) {
    boost::container::flat_map<K, V>().swap(map);
  }

  // boost::container::map
  template<typename K, typename V>
  void destroy(boost::container::map<K, V>& map) {
    boost::container::map<K, V>().swap(map);
  }

  inline bool is_answer_yes(const std::string &answer) {
    std::string YES;
    for (const auto &c:answer) {
      YES += std::tolower(c);
    }

    return YES == "y" || YES == "yes" || YES == "yeesss";
  }


WYLAND_END