#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <new>

/* Runtime */
#include "wylrt.h"
#include "libcallc.hpp"
/* Targets */
#include "targets.h"
#include "wtarget64.hpp"
#include "wtarget32.hpp"
#include "wtargetfast.hpp"

#include "regs.hpp"
#include "wformat.hpp"
#include "wmmbase.hpp"
#include "wtypes.h"
#include "wfiles.h"
#include "wtargb.hpp"
#include "wyland.h"
#include "wyland.hpp"

#include "sock2.h"

WYLAND_BEGIN

typedef void (*taskHandle)(std::vector<std::string>&);

typedef struct {
  __wtarget   target;
  uint32_t    version;
  /* In the future.. */
} rt_task_t;

rt_task_t task{wtarg64, WYLAND_VERSION_UINT32};

taskHandle name = [](std::vector<std::string>&) {
  std::cout << WYLAND_NAME << std::endl;
};

taskHandle version = [](std::vector<std::string>&) {
  std::cout << WYLAND_VERSION << std::endl;
};

taskHandle build = [](std::vector<std::string>&) {
  std::cout << WYLAND_BUILD << std::endl;
};

taskHandle target = [](std::vector<std::string>&) {
  std::cout << wtarg64 << ": " << nameof(wtarg64) << std::endl;
};

taskHandle target_info = [](std::vector<std::string>&) {
  std::cout << wtarg64 << ": " << nameof(wtarg64) << " (implemented)" << std::endl;
  std::cout << wtarg32 << ": " << nameof(wtarg32) << " (working on)" << std::endl;
  std::cout << wtargmarch << ": " << nameof(wtargmarch) << " (working on)" << std::endl;
  std::cout << wtargfast << ": " << nameof(wtargfast) << " (working on)" << std::endl;
};

taskHandle set_target = [](std::vector<std::string> &args) {
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> target after -target token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
    std::cerr << "[w]: Too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  } 

  task.target = ofname(args[0].c_str());
};

taskHandle run = [](std::vector<std::string> &args) {
  bool auto_targ = false;

  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> disk after -run token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
    if (args[2] == "-auto") auto_targ = true;

    std::cerr << "[w]: Too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  }

  std::fstream disk(args[0]);

  if (!disk) {
    std::cerr << "[e]: Unable to open disk file: " << args[0] << std::endl;
    exit(-1);
  }

  wblock *block = new wblock;
  disk.read((char*)block->array, sizeof(block->array));
  auto header = wyland_files_make_header(block);

  if (auto_targ) task.target = header.target;

  if (!wyland_files_parse(&header, task.target, task.version)) {
    std::cerr << "[e]: " << std::invalid_argument("Invalid header file.").what() << std::endl;
    std::cout << "Extracted header:\n" << wyland_files_header_fmt(&header) << std::endl;

    exit(-1);
  }

  delete block;

  core_base *core = create_core_ptr(task.target);
  
  if (!load_file(disk, header)) {
    delete core;
    exit(-1);
  }

  std::cout << "[i]: initializing object 0x" << std::hex << reinterpret_cast<uintptr_t>(core) << std::endl;
  core->init(SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START+SYSTEM_SEGMENT_SIZE, true, 0);
  run_core(core);

  delete core;
};

taskHandle run_raw = [](std::vector<std::string> &args) {
  std::cerr << "[w]: Running -run-raw mode." << std::endl;
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> target after -target token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
    std::cerr << "[w]: Too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  }

  std::fstream disk(args[0]);

  if (!disk) {
    std::cerr << "[e]: Unable to open disk file: " << args[0] << std::endl;
    exit(-1);
  } else { 
    std::cout << "[i]: Disk opened." << std::endl;
  }

  size_t i = 0;
  while (!disk.eof() && i < SYSTEM_SEGMENT_SIZE) {
    char buff[1]{0};
    disk.read(buff, sizeof(buff));
    memory[SYSTEM_SEGMENT_START+i] = buff[0];
    i++;
  }

  core_base *core = create_core_ptr(task.target);

  if (core == nullptr) {
    std::cerr << "[e]: *core is a bad pointer." << std::endl;
    exit(-400);
  }

  std::cout << "[i]: initializing object 0x" << std::hex << reinterpret_cast<uintptr_t>(core) << std::endl;
  core->init(SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START+SYSTEM_SEGMENT_SIZE, true, 0);
  run_core(core);

  delete core;
};

taskHandle check = [](std::vector<std::string> &args) {
  for (size_t i = 0; i < args.size(); i++) {
    if (args[i] == "--raw") {
      std::ifstream disk(args[++i]);
      wblock *block = new wblock;

      disk.read((char*)block->array, sizeof(block->array));

      for (int i = 0; i < sizeof(block->array); i++) std::cout << (int)block->array[i];
      std::cout << std::endl;
      disk.close();
      delete block;
    }

    if (args[i] == "--spec") {
      std::ifstream disk(args[++i]);
      wblock *block = new wblock;

      disk.read((char*)block->array, sizeof(block->array));
      std::cout << wyland_files_fmt_header_spec(block, '\n') << std::endl;
      disk.close();
      delete block;
    }

    std::ifstream disk(args[i]);

    if (!disk) std::cerr << "[e]: not a file: " << args[i] << std::endl;

    wblock *block = new wblock;
    disk.read((char*)block->array, sizeof(block->array));
    auto header = wyland_files_make_header(block);
    
    std::cout << wyland_files_header_fmt(&header) << std::endl;
  }
};

taskHandle make_disk = [](std::vector<std::string> &args) {
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Excepted disk name.").what() << std::endl;
    exit(-1);
  }

  std::ofstream disk(args[0]);
  
  if (!disk) {
    std::cerr << "[e]: Unable to create file " << args[0] << std::endl;
    exit(-1);
  }

  wheader_t header = wyland_files_basic_header();
  
  for (size_t i = 1; i < args.size(); i++) {
    if (args[i] == "-version") {
      header.version = std::stoul(args[++i]);
    } else if (args[i] == "-target") {
      header.version = ofname(args[++i].c_str());
    } else if (args[i] == "-data") {
      header.data = std::stoull(args[++i]);
    } else if (args[i] == "-code") {
      header.code = std::stoull(args[++i]);
    } else if (args[i] == "-lib") {
      header.lib = std::stoull(args[++i]);
    }  
  }
  
  disk.write((char*)wyland_files_header_to_block(&header).array, sizeof(wblock));

  for (size_t i = 1; i < args.size(); i++) {
    if (args[i].starts_with("-")) {
      i++;
      continue;
    }

    std::ifstream file(args[i]);
    if (!file) {
      std::cerr << "[e]: Unable to create file " << args[i] << std::endl;
      exit(-1);
    }

    while (!file.eof()) {
      char buff[4]{0};
      file.read(buff, sizeof(buff));
      disk.write(buff, sizeof(buff));
    }

    file.close();
  }

  disk.close();

  std::cout << wyland_files_header_fmt(&header) << std::endl;
  std::cout << "Saved to: " << args[0] << std::endl;
};

std::unordered_map<std::string, taskHandle> handles {
  {"--v", version},
  {"--version", version},
  {"--n", name},
  {"--name", name},
  {"--b", build},
  {"--build", build},
  {"--target", target},
  {"--target-info", target_info},
  {"-check", check},
  {"-target", set_target},
  {"-run", run},
  {"-run-raw", run_raw},
  {"-make-disk", make_disk},
  {"-build-disk", make_disk}, 
  //{"-parse", parse},
  //{"-debug", debug},
  //{"-new-env", new_env},
  // Future tasks (Maybe in the std:wy4 standard !)
  //{"-compile", compile},
  //{"-libsof", libsof},
  //{"-api", api}
};

int wylandMain(int argc, char *const argv[]) {
  if (argc - 1 == 0) {
    std::cerr << "[e]: Expected a task." << std::endl;
    return -1;
  }

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (handles.find(arg) != handles.end()) {
      std::vector<std::string> args;

      int j = i + 1;
      while (j < argc) {
        args.push_back(argv[j]);
        j++;
      }

      handles[arg](args);

      i = j - 1;
    } else {
      std::cerr << "[e]: Unknown argument: " << arg << std::endl;
      return -1;
    }
  }

  return 0;
}

WYLAND_END

int main(int argc, char *const argv[]) {
  return wylma::wyland::wylandMain(argc, argv);
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
  wtargfast: NON-STANDARD ! The fastest (jumps are limited to: jmp, je, jg and jl)

  Pay attention ! 
  -run <disk>, here, <disk> is not an input file like an OS. It's a disk !
  In a disk, you can find multiple informations, 
  such as the target (or targets, and adress of each target's code), 
  version, libs and important virtual addresses.
*/