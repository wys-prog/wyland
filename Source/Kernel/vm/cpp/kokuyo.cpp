#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "kokuyo.hpp"

/* args
  -i <file> -- Define input
  -inc <libname> <function name> -- will import the function
  -inc <libname> /all -- will import all functions
 */

class vm_handle {
private:
  kokuyo::kokuyoVM vm;
  std::vector<std::string> argv;
  std::vector<std::string> inputs;

public:
  void handle() {
    for (size_t i = 0; i < argv.size(); i++) {
      if (argv[i] == "-i") inputs.push_back(argv[++i]);
      else if (argv[i] == "-inc") {
        /* inc <WHAT?> <WHERE?> */
        std::string what = argv[i++], where = argv[i++];
        
        if (what == "/all") {}
      }
    }
  }
};

int main(int argc, char const *argv[]) {
  kokuyo::kokuyoVM vm;

  return 0;
}
