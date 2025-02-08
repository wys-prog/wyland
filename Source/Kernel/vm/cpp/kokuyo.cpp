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
  std::function<void()> tocall = [](){return;};

  void handle_inputs() {
    std::ifstream file(input);
    if (!file) {
      tocall = [&]() {throw std::runtime_error("Unable to open input: " + input); };
      return;
    }

    while (!file.eof()) {
      char buff[1] = {0};
      file.read(buff, 1);

      program.push_back(buff[0]);
    }

    file.close();
  }

  void printstack() {
    for (size_t i = 0; i < vm.get_trace().size(); i++) {
      std::cout << i << ": " << vm.get_trace()[i] << std::endl;
    } 
  }

public:
  void handle() {
    std::unordered_map<std::string, LibHandle> handles;

    for (size_t i = 0; i < argv.size(); i++) {
      if (argv[i] == "-i") input = argv[++i];
      else if (argv[i] == "-inc") {
        /* inc <WHAT?> <WHERE?> */
        std::string what = argv[i++], where = std::filesystem::absolute(argv[i++]);

        if (handles.find(where) == handles.end()) 
          handles[where] = dynlib_open(where.c_str());
        std::function<uint64_t(uint64_t, uint8_t*)> myfunc = (*dynlib_get_function(handles[where], what.c_str()));

        vm.append_function(what, myfunc);
        std::cout << "Added function " << what << " from " << where << std::endl; 
      } else if (argv[i] == "-incs") {
        std::vector<std::string> whats;
        std::string where, tmp;

        while (tmp != "/") whats.push_back(argv[i++]);
        where = argv[i++];

        if (handles.find(where) == handles.end()) 
          handles[where] = dynlib_open(where.c_str());

        for (const auto &what : whats) {
          std::function<uint64_t(uint64_t, uint8_t*)> myfunc = (*dynlib_get_function(handles[where], what.c_str()));
          vm.append_function(what, myfunc);
          std::cout << "Added function " << what << " from " << where << std::endl; 
        }
      }
    }
    
    for (const auto &handle : handles) dynlib_close(handle.second);

    argv.clear(); /* Free memory, don’t need anymore. */
    handle_inputs();
  }

  int run() {
    int ext = 0;
    
    try {
      tocall();
      if (!std::filesystem::exists(input)) throw std::runtime_error("No such file: " + input);
      vm.invoke(program);
    } catch(const std::exception &e) {
      vm.get_trace()
      .push_back(typeid(e).name() + std::string(" what():\t") + std::string(e.what()));
      ext++;
    }

    printstack();
    return ext;
  }

  vm_handle(int argc, char const *_argv[]) {
    for (int i = 1; i < argc; i++) {
      argv.push_back(_argv[i]);
    }
  }

  size_t size() {
    return sizeof(this) + input.capacity() + program.capacity(); + argv.capacity();
  }
};

int main(int argc, char const *argv[]) {
  std::cout << "Kokuyo/Wylma Virtual Machine - 1.0" << std::endl;
  vm_handle handle(argc, argv);

  handle.handle();
  std::cout << "Sizeof handle: " << handle.size() << " bytes" << std::endl;
  std::cout << "Invoking core..." << std::endl;
  
  return handle.run();
}
