#pragma once

#include <iostream>
#include <unordered_map>

#define RESET     "\033[0m"
#define BLACK     "\033[30m"     
#define RED       "\033[31m"     
#define GREEN     "\033[32m"     
#define YELLOW    "\033[33m"     
#define BLUE      "\033[34m"     
#define MAGENTA   "\033[35m"     
#define CYAN      "\033[36m"     
#define WHITE     "\033[37m"     
#define BOLD      "\033[1m"
#define UNDERLINE "\033[4m"
#define REVERSE   "\033[7m"

namespace wyland {
  class stream_writer {
  private:
    std::unordered_map<std::string, std::ostream*> outputs;
    uint64_t calls = 0;

  public: 
    // Constructor allows adding multiple streams with names
    stream_writer(std::initializer_list<std::pair<std::string, std::ostream*>> streams) {
      for (const auto& [name, stream] : streams) {
        if (stream) outputs[name] = stream;
      }
    }

    // Add an output stream dynamically
    void add_stream(const std::string& name, std::ostream& stream) {
      outputs[name] = &stream;
    }

    // Remove a stream by name
    void remove_stream(const std::string& name) {
      outputs.erase(name);
    }

    // Generic logging function
    template <typename... Args>
    void log(Args&&... args) {
      for (auto& [name, out] : outputs) {
        (*out) << calls++ << ": ";
        ((*out) << ... << args) << RESET << std::endl;
      }
    }

    // Specific logging functions
    template <typename... Args>
    void error(Args&&... args) { log("[e]: ", std::forward<Args>(args)...); }

    template <typename... Args>
    void warn(Args&&... args) { log("[w]: ", std::forward<Args>(args)...); }

    template <typename... Args>
    void info(Args&&... args) { log("[i]: ", std::forward<Args>(args)...); }
  };
} // namespace wyland
