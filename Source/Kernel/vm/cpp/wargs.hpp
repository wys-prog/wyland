#pragma once

#include <vector>
#include <string>
#include <cctype>


namespace wyland {
  std::vector<std::string> extract_args(const std::string &cmd) {
    std::vector<std::string> args;
    std::string buff;
    bool quoted = false, dquoted = false;
  
    for (size_t i = 0; i < cmd.length(); ++i) {
      char c = cmd[i];
  
      if (c == '"' && !quoted) { 
        dquoted = !dquoted;
        if (dquoted) continue; // Ignore opening quote
      } 
      else if (c == '\'' && !dquoted) {
        quoted = !quoted;
        if (quoted) continue; // Ignore opening quote
      }
      else if (!quoted && !dquoted && isspace(c)) {
        if (!buff.empty()) {
          args.push_back(buff);
          buff.clear();
        }
      }
      else {
        buff += c;
      }
    }
  
    if (!buff.empty()) args.push_back(buff); // Push last argument
  
    return args;
  }

  std::string build_command(const std::string &name, const std::vector<std::string> &argv) {
    std::string buff = name + " ";
    for (const auto &arg : argv) buff += arg + " ";
    return buff;
  }
} // namespace wyland
