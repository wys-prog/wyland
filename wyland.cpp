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
#include "wfiles.h"
#include "wtargb.hpp"
#include "wyland.h"
#include "wyland.hpp"

typedef void (*taskHandle)(const std::vector<std::string>&);

typedef struct {
  __wtarget   target;
  uint32_t    version;
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
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("expected <x> target after -target token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
    std::cerr << "[w]: too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  } 

  task.target = ofname(args[0].c_str());
};

taskHandle run = [](const std::vector<std::string> &args) {
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("expected <x> target after -target token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
    std::cerr << "[w]: too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  }

  std::fstream disk(args[0]);

  if (!disk) {
    std::cerr << "[e]: unable to open disk file: " << args[0] << std::endl;
    exit(-1);
  }

  wblock *block = new wblock;
  disk.read((char*)block->array, sizeof(block->array));
  auto header = wyland_files_make_header(block);

  if (!wyland_files_parse(&header, task.target, task.version)) {
    std::cerr << "[e]: " << std::invalid_argument("invalid header file.").what() << std::endl;
    exit(-1);
  }

  delete block;

  core_base *core = create_core_ptr(task.target);
  load_file(disk, header);
  core->init(SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START+SYSTEM_SEGMENT_SIZE, true, 'S'+'y'+'s'+'t'+'e'+'m');
  run_core(core);

  delete core;
};

std::unordered_map<std::string, taskHandle> handles;

int main(int argc, char *const argv[]) {
  if (argc - 1 == 0) {
    std::cerr << "[e]: excepted a task." << std::endl;
    return -1;
  }

  for (int i = 1; i < argc; i++) {
    if (handles.find(std::string(argv[i])) != handles.end()) {
      std::vector<std::string> args;
      for (int j = i; j < argc; j++) args.push_back(argv[i]);
      handles[std::string(argv[i])](args);
    } else {
      std::cerr << "[e]: unknown argument: " << argv[i] << std::endl;
      return -1;
    }
  }

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