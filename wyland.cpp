#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <new>

#include "regs.hpp"
#include "wylrt.h"
#include "libcallc.hpp"
#include "targets.h"
#include "wtarget64.hpp"
#include "wformat.hpp"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wyland.h"

typedef void (*taskHandle)(const std::vector<std::string>&);

typedef struct {
  std::string source;
  __wtarget   target;
  /* In the future.. */
} rt_task_t;

rt_task_t task;

taskHandle name = [](const std::vector<std::string>&) {
  std::cout << WYLAND_NAME << std::endl;
};

taskHandle version = [](const std::vector<std::string>&) {
  std::cout << WYLAND_VERSION << std::endl;
};

taskHandle build = [](const std::vector<std::string>&) {
  std::cout << WYLAND_BUILD << std::endl;
};

taskHandle target = [](const std::vector<std::string>&) {
  std::cout << wtarg64 << ": " << nameof(wtarg64) << std::endl;
};

taskHandle target_info = [](const std::vector<std::string>&) {
  std::cout << wtarg64 << ": " << nameof(wtarg64) << std::endl;
  std::cout << wtarg32 << ": " << nameof(wtarg32) << std::endl;
  std::cout << wtargmarch << ": " << nameof(wtargmarch) << std::endl;
  std::cout << wtargfast << ": " << nameof(wtargfast) << std::endl;
};

taskHandle set_target = [](const std::vector<std::string> &args) {
  if (args.size() < 0) {
    std::cerr << "[e]: " << std::invalid_argument("expected <x> target after -target token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
    std::cerr << "[w]: too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  } 

  task.target = ofname(args[0].c_str());
};

taskHandle run = [](const std::vector<std::string> &args) {
  if (args.size() < 0) {
    std::cerr << "[e]: " << std::invalid_argument("expected <x> target after -target token.").what() << std::endl;
    exit(-1);
  }

  
};

int main(int argc, char *const argv[]) {

  return 0;
}

/* Arguments: 
  --v, --version:  prints the version
  --n, --name:     prints the name
  --b, --build:    prints the build name
  --target:        prints the default target
  --target-info:   prints informations about targers
  --check:         finds all files in .disk in ./ and ../        

  -target <x>:     sets the target to x
  -run <file>:     run <file> 
  -parse <file>:   parses the <file>.
  -debug <file>:   debugs a <file>
  -new-env <name>: creates a new environnement 

  --- in the future --- 
  -compile <file>: compiles the <file>
  -libsof <file>:  returns libs in <file>
  -api <lib>:      tries to load <lib> as library

  --- targets ---
  wtarg64: Basic Target 
  wtarg32: Basic Target in 32 bits mode
  wtargmarch: Target built for Mathematics Operations, and big floats.
  wtargfast: NON-STANDARD ! The fastest (jumps are limited to: jmp, je)

  Pay attention ! 
  -run <disk>, here, <disk> is not an input file like an OS. It's a disk !
  In a disk, you can find multiple informations, 
  such as the target (or targets, and adress of each target's code), 
  version, libs and important virtual addresses.
*/