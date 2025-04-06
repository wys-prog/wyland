#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <cctype>

#include "wyland.hpp"

WYLAND_BEGIN

size_t get_to_alloc(const std::string &arg) {
  size_t mem_pos = arg.find("-memory:");
  if (mem_pos == std::string::npos) return WYLAND_MEMORY_MINIMUM;

  std::string size_part = arg.substr(mem_pos + 8); 

  size_t i = 0;
  while (i < size_part.size() && std::isdigit(size_part[i])) i++;

  if (i == 0) return WYLAND_MEMORY_MINIMUM; 

  std::string num_str = size_part.substr(0, i);
  std::string unit_str = size_part.substr(i);

  for (auto &c : unit_str) c = std::tolower(c);

  try {
    uint64_t value = std::stoull(num_str);

    if (unit_str == "gb") return value * 1024 * 1024 * 1024;
    if (unit_str == "mb") return value * 1024 * 1024;
    if (unit_str == "kb") return value * 1024;
    if (unit_str == "b" || unit_str == "") return value;

    return WYLAND_MEMORY_MINIMUM;
  } catch (const std::out_of_range &) {
    std::cerr << "[e]: " << num_str << ": too large value. defaulting to: "
              << WYLAND_MEMORY_MINIMUM << " bytes." << std::endl;
    return WYLAND_MEMORY_MINIMUM;
  } catch (const std::exception &) {
    return WYLAND_MEMORY_MINIMUM;
  }
}

WYLAND_END
