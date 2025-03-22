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
  std::cout << wtarg64 << ": " << nameof(wtarg64) << std::endl;
  std::cout << wtarg32 << ": " << nameof(wtarg32) << std::endl;
  std::cout << wtargmarch << ": " << nameof(wtargmarch) << std::endl;
  std::cout << wtargfast << ": " << nameof(wtargfast) << std::endl;
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
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> disk after -run token.").what() << std::endl;
    exit(-1);
  } else if (args.size() > 1) {
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

  if (!wyland_files_parse(&header, task.target, task.version)) {
    std::cerr << "[e]: " << std::invalid_argument("Invalid header file.").what() << std::endl;
    exit(-1);
  }

  delete block;

  core_base *core = create_core_ptr(task.target);
  load_file(disk, header);
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
  for (const auto &arg:args) {
    std::ifstream disk(arg);

    if (!disk) std::cerr << "[e]: not a file: " << arg << std::endl;

    wblock *block = new wblock;
    disk.read((char*)block->array, sizeof(block->array));
    auto header = wyland_files_make_header(block);
  
    
    std::cout << "certificat:\t" << (char*)header.certificat << "\n"
                 "target:\t" << header.target << " - " << nameof(header.target) << "\n"
                 "version:\t" << header.version << "\n"
                 "code start:\t" << std::hex << header.code << "\n"
                 "data start:\t" << header.data << "\n"
                 "lib. start:\t" << header.lib << "\n"
    << std::endl;
    
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
  disk.write((char*)wyland_files_header_to_block(&header).array, sizeof(wblock));

  for (size_t i = 0; i < args.size(); i++) {
    if (args[i] == "-version") {
      header.version = std::stoul(args[++i]);
    } else if (args[i] == "-target") {
      header.version = std::stoul(args[++i]);
    }
  }

  for (size_t i = 1; i < args.size(); i++) {
    std::ifstream file(args[i]);
    if (!file) {
      std::cerr << "[e]: Unable to create file " << args[i] << std::endl;
      exit(-1);
    }

    while (!file.eof()) {
      char buff[64]{0};
      file.read(buff, sizeof(buff));
      disk.write(buff, sizeof(buff));
    }

    file.close();
  }

  disk.close();
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
  //{"-parse", parse},
  //{"-debug", debug},
  //{"-new-env", new_env},
  // Future tasks
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
      while (j < argc && handles.find(argv[j]) == handles.end()) {
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