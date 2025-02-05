#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "kokuyo.hpp"
#include "dlapi.h"

/* args
  -i <file> -- Define input
  -inc <libname> <function name> -- will import the function
 */

class vm_handle {
private:
  kokuyo::kokuyoVM vm;
  std::vector<std::string> argv;
  std::vector<uint8_t> program;
  std::string input;

  void handle_inputs() {
    std::ifstream file(input);
    if (!file) throw std::runtime_error("Unable to open input: " + input);

    while (!file.eof()) {
      char buff[1] = {0};
      file.read(buff, 1);

      program.push_back(buff[0]);
    }

    file.close();
  }

public:
  void handle() {
    std::unordered_map<std::string, LibHandle> handles;

    for (size_t i = 0; i < argv.size(); i++) {
      if (argv[i] == "-i") input = argv[i];
      else if (argv[i] == "-inc") {
        /* inc <WHAT?> <WHERE?> */
        std::string what = argv[i++], where = std::filesystem::absolute(argv[i++]);

        if (handles.find(where) == handles.end()) 
          handles[where] = dynlib_open(where.c_str());
        std::function<uint64_t(uint8_t*)> myfunc = (*dynlib_get_function(handles[where], what.c_str()));

        vm.append_function(what, myfunc);
        std::cout << "Added function " << what << " from " << where << std::endl; 
      }
    }
    
    for (const auto &handle : handles) dynlib_close(handle.second);

    argv.clear(); /* Free memory, don’t need anymore. */
    handle_inputs();
  }

  void run() {
    vm.invoke(program);
  }

  vm_handle(int argc, char const *_argv[]) {
    for (int i = 1; i < argc; i++) {
      argv.push_back(_argv[i]);
    }
  }
};

int main(int argc, char const *argv[]) {
  vm_handle handle(argc, argv);

  handle.handle();
  handle.run();

  return 0;
}
