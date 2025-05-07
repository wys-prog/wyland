#pragma once

#include <iostream>
#include <stdexcept>
#include <exception>
#include <typeinfo>

#include "wyland-runtime/wylrt.hpp"

WYLAND_BEGIN

#define wthrow throw

// Specialized version for wyland_runtime_error
[[noreturn]] void Nwthrow(const runtime::wyland_runtime_error &exception) {
  std::cerr << "[e]: WylandC++ Exception:\t" << exception.fmterr() << std::endl;
  std::terminate();
}

// Specialized version for standard exceptions
[[noreturn]] void Nwthrow(const std::exception &e) {
  std::cerr << "[e]: WylandC++ Exception:\t" << e.what() << std::endl;
  std::terminate();
}

WYLAND_END
