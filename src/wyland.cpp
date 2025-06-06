#ifdef _WIN32
#include <windows.h>
#include <oleauto.h>
#endif 

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
#include <vector> 
#include <thread>
#include <chrono>
#include <mutex>
#include <new>

#ifdef WYLAND_STACKTRACE
//#include <boost/stacktrace.hpp>
#endif // ? WYLAND_STACKTRACE

#include <boost/container/flat_map.hpp>

#include "data/wyland_version.h"
#include "wyland_config.hpp"

#include "wmutiles.hpp"

/* Runtime */
#include "wyland-runtime/wylrt.h"
#include "wyland-runtime/wylrt.hpp"
#include "libcallc.hpp"
/* Targets */
#include "targets.h"
#include "wtarget64.hpp"
#include "wtarget32.hpp"
#include "wtargmarch.hpp"
#include "wtargetfast.hpp"

#include "bios/bios.hpp"
#include "bios/bios_backend.hpp"
#include "cache.hpp"
#include "compiler/iwysm.hpp"
#include "filestream.hpp"
#include "parser.hpp"
#include "regs.hpp"
#include "security.hpp"
#include "sock2.h"
#include "system.hpp"
#include "updates/updater.hpp"
#include "wc++std.hpp"
#include "wfiles.h"
#include "wformat.hpp"
#include "wmmbase.hpp"
#include "wtargb.hpp"
#include "wtypes.h"
#include "wyland.h"
#include "wyland.hpp"

#include "bios/bios.hpp"
#include "bios/bios_usb.hpp"
#include "bios/bios_disk.hpp"
#include "bios/stdusb/stdusb.hpp"

WYLAND_BEGIN

typedef void (*TaskHandle)(std::vector<std::string>&);

typedef struct {
  warch_t     target;
  uint32_t    version;
  bool        auto_targ;
  uint64_t    memory;
  std::string GraphicsModulePath;
  std::string Module1Path;
  std::string Module2Path;
  int         max_cycles;
  bool        format_libs_name;
  std::vector<std::string> usb_devices;
  wuint       usb_ports;
  /* In the future.. */
} rt_task_t;

rt_task_t task {
  .target = warch64, 
  .version = WYLAND_VERSION_UINT32, 
  .auto_targ = false, 
  .memory = WYLAND_MEMORY_MINIMUM,
  .GraphicsModulePath = "",
  .Module1Path = "", 
  .Module2Path = "",
  .max_cycles = -1,
  .format_libs_name = true,
  .usb_devices{}, 
  .usb_ports = WYLAND_STD_USB_PORTSC
};

/* This is idk (for fun) */
template <typename Rep, typename Period>
struct swaiter {
  std::chrono::duration<Rep, Period> duration;
}; /* Streamer Waiter */


template <typename Rep, typename Period>
std::ostream &operator<<(std::ostream &os, const swaiter<Rep, Period> &w) {
  std::this_thread::sleep_for(w.duration);
  return os;
}

wuint towuint(const std::string &str, wuint default_value = 0) {
  try {
    return std::stoul(str);
  } catch (const std::exception &e) {
    std::cerr << "[e]: towuint(): failed: " << e.what() << " [with str:" << str << "]" << std::endl;
    return default_value;
  }
}

void handle_arguments(std::vector<std::string> args, std::vector<std::string> &files) {
  for (size_t i = 0; i < args.size(); i++) {
    args[i] = trim(args[i]);
    if (args[i] == "-auto") task.auto_targ = true;
    else if (args[i] == "-target" || args[i] == "-arch") {
      if (args.size() <= i + 1) { std::cerr << "[e]: excepted argument after " << args[i] << std::endl; wyland_exit(-1); }
      task.target = ofname(args[++i].c_str());
      std::cout << "[i]: arch. set to: " << task.target << " (" << args[i] << ")" << std::endl;
    } else if (args[i].starts_with("-memory:")) {
      task.memory = static_cast<uint64_t>(get_to_alloc(args[i]));
    } else if (args[i] == "-GraphicsModule" || args[i] == "-gm") {
      if (args.size() <= i + 1) { std::cerr << "[e]: excepted argument after " << args[i] << std::endl; wyland_exit(-1); }
      task.GraphicsModulePath = args[++i];
    } else if (args[i] == "-module1" || args[i] == "-m1") {
      if (args.size() <= i + 1) { std::cerr << "[e]: excepted argument after " << args[i] << std::endl; wyland_exit(-1); }
      task.Module1Path = args[++i];
    } else if (args[i] == "-module2" || args[i] == "-m2") {
      if (args.size() <= i + 1) { std::cerr << "[e]: excepted argument after " << args[i] << std::endl; wyland_exit(-1); }
      task.Module2Path = args[++i];
    } else if (args[i] == "-max" || args[i] == "-max-cycles") {
      if (args.size() <= i + 1) { std::cerr << "[e]: expected argument after " << args[i] << std::endl; wyland_exit(-1); }
      try { 
        task.max_cycles = std::stoi(args[++i]); 
        std::cout << "[i]: max cycle count set to: " << std::dec << task.max_cycles << std::endl;
      } catch (...) {
        std::cerr << "[e]: invalid format for " << args[i] << std::endl;
        wyland_exit(-1);
      }
    } else if (args[i] == "-fmt-libs-false" || args[i] == "-nofmtlibs") {
      task.format_libs_name = false;
    } else if (args[i] == "-no-init-out" || args[i] == "-niout" || args[i] == "-no-v") {
      std::cout.setstate(std::ios_base::failbit);
    } else if (args[i] == "-usb") {
      if (args.size() <= i + 1) { std::cerr << "[e]: excepted argument after " << args[i] << std::endl; wyland_exit(-1); }
      task.usb_devices.push_back(args[++i]);
    } else if (args[i].starts_with("-usb-ports:")) {
      size_t beg = args[i].find("-usb-ports:");
      task.usb_ports = towuint(args[i].substr(beg + 11), WYLAND_STD_USB_PORTSC);
    } else {
      files.push_back(args[i]);
    }
  }
  for (wuint i = 0; i < task.usb_ports; i++) task.usb_devices.emplace_back("_wdefault");
}

void run_base_function(std::vector<std::string> &args, bool debug = false) {
  cache::init_cache();
  if (args.empty()) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> disk after -run token.").what() << std::endl;
    wyland_exit(-1);
  }

  std::vector<std::string> files;
  handle_arguments(args, files);

  if (!debug && task.max_cycles != -1) std::cerr << "[w]: ignoring -max argument: only in debug mode" << std::endl;

  for (const auto&file:files) {
    std::fstream disk(file);

    if (!disk) {
      std::cerr << "[e]: Unable to open disk file: " << file << std::endl;
      wyland_exit(-1);
    }
  
    auto *block = new wblock;
    disk.read(reinterpret_cast<char*>(block->array), sizeof(block->array));
    auto header = wyland_files_make_header(block);
    delete block;
    w_dfstream stream(disk, header.data); // TODO: Adding enabling/disabling of "append/extend disk's size"
    
    if (task.auto_targ) task.target = header.target;
    
    if (!wyland_files_parse(&header, task.target, task.version)) {
      std::cerr << "[e]: " << std::invalid_argument("Invalid header file.").what() << std::endl;
      std::cout << "Extracted header:\n" << wyland_files_header_fmt(&header) << std::endl;
      
      wyland_exit(-1);
    }
    
    load_libs(disk, header.data, header, task.format_libs_name);
    
    wyland_base_core *core = create_core_ptr(task.target);
    allocate_memory(task.memory);
    loadModules(task.GraphicsModulePath, task.Module1Path, task.Module2Path, stream);
    loadUSBDevices(task.usb_devices);
    
    if (!load_file(disk, header)) {
      delete core;
      wyland_exit(-1);
    }
    
    if (core) {
      std::cerr << "[e]: *core is a bad pointer." << std::endl;
      wyland_exit(-400);
    }

    std::cout << "[i]: initializing object 0x" << std::hex << reinterpret_cast<uintptr_t>(core) << std::endl;
    core->init(
      SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START+SYSTEM_SEGMENT_SIZE, 
      true, 0, &cache::linked_funcs, SYSTEM_SEGMENT_START+code_start, cache::GraphicsModulePtr, 
      cache::MMIOModule1Ptr, cache::MMIOModule2Ptr, cache::DiskModulePtr, cache::BiosPtr
    );
    
    run_core(core, debug, task.max_cycles);
  
    delete core;
    
    disk.close();
    wyland_exit();
  }
}

TaskHandle name = [](std::vector<std::string>&) {
  std::cout << WYLAND_NAME << std::endl;
};

TaskHandle version = [](std::vector<std::string>&) {
  std::cout << WYLAND_VERSION << std::endl;
};

TaskHandle build = [](std::vector<std::string> &) {
  std::string string = WYLAND_BUILD_NAME;
  for (const auto&c:string) std::cout << c << swaiter{5ms} << std::flush;
  std::cout << std::endl;
};

TaskHandle target = [](std::vector<std::string>&) {
  std::cout << warch64 << ": " << nameof(warch64) << std::endl;
};

TaskHandle target_info = [](std::vector<std::string>&) {
  std::cout << warch64 << ": " << nameof(warch64) << " (implemented)" << std::endl;
  std::cout << wtarg32 << ": " << nameof(wtarg32) << " (working on)" << std::endl;
  std::cout << wtargmarch << ": " << nameof(wtargmarch) << " (working on)" << std::endl;
  std::cout << wtargfast << ": " << nameof(wtargfast) << " (working on)" << std::endl;
};

TaskHandle infos = [](std::vector<std::string> &args) {

  // I added "swaiter{10ms}" just for make fun and a bit "old" style...
  // by doing --info --nw, wait time will set to 0
  swaiter wait{10ms};
  for (const auto &arg:args) {
    if (arg == "--nw" || arg == "--no-wait") {
      wait.duration = 0ms;
    }
  }

  std::stringstream ss;

  ss << "name:\t\t" << WYLAND_NAME << "\n" 
            << "version:\t" << WYLAND_VERSION "(" << WYLAND_VERSION_UINT32 << ")\n" << swaiter{10ns}
            << "build:\t\t" << WYLAND_BUILD_NAME << "\n"  
            << "targets:\t\n"  <<
              "\t- warch64\t(" << (warch64) << ")\n"  <<
              "\t- wtarg32\t(" << (wtarg32) << ")\n"  <<
              "\t- wtargmarch\t(" << (wtargmarch) << ")\n" <<
              "\t- wtargfast\t(" << (wtargfast) << ")\n"
              "===== SYSTEM =====\n"
              << os.get_fmt_specs()
              << "===== RUNTIME =====\n"
              "runtime version:\t" << wyland_get_runtime_version() << "\n"
              "runtime compiler:\t" << wyland_get_runtime_compiler() << "\n"
              "sizeof base-runtime:\t" << sizeof(wyland_base_core) << "\n"
              "sizeof warch64:\t\t" << sizeof(corewtarg64) << "\n"
              "sizeof warch128:\t" << sizeof(core_warch128) << "\n"
              "runtime type:\t\t" WYLAND_BUILD_MODE "\n"
              "===== BIOS =====\n"
              "BIOS version:\t" << (bios_backend_version()) << "\n" 
              "===== COMPILER =====\n" 
              "Compiler version:\t" << wysm::WylandAssembler::version
            << std::endl;
  
  std::string line;
  while (std::getline(ss, line)) std::cout << line << wait << std::endl;
};

TaskHandle set_target = [](std::vector<std::string> &args) {
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> target after -target token.").what() << std::endl;
    wyland_exit(-1);
  } else if (args.size() > 1) {
    std::cerr << "[w]: Too much arguments (" << args.size() << "). Excepted 1." << std::endl;
  } 

  task.target = ofname(args[0].c_str());
};

TaskHandle run = [](std::vector<std::string> &args) {
  run_base_function(args, false);
};

TaskHandle run_raw = [](std::vector<std::string> &args) {
  cache::init_cache();
  std::cerr << "[w]: Running -run-raw mode." << std::endl;
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Expected <x> target after -target token.").what() << std::endl;
    wyland_exit(-1);
  }

  std::vector<std::string> files;
  handle_arguments(args, files);

  for (const auto&file:files) {
    std::fstream disk(file);
    w_dfstream stream(disk, 0);

    if (!disk) {
      std::cerr << "[e]: Unable to open disk file: " << file << std::endl;
      wyland_exit(-1);
    } else { 
      std::cout << "[i]: Disk opened." << std::endl;
    }

    allocate_memory(task.memory);
    loadModules(task.GraphicsModulePath, task.Module1Path, task.Module2Path, stream);
    loadUSBDevices(task.usb_devices);

    size_t i = 0;
    while (!disk.eof() && i < SYSTEM_SEGMENT_SIZE) {
      char buff[1]{0};
      disk.read(buff, sizeof(buff));
      memory[SYSTEM_SEGMENT_START+i] = buff[0];
      i++;
      std::cout << "[i]: loaded: " << std::dec << i << " bytes...\r" << std::flush;
    }
    std::cout << "[i]: loaded: " << std::dec << i << " bytes" << std::endl;

    wyland_base_core *core = create_core_ptr(task.target);

    if (core == nullptr) {
      std::cerr << "[e]: *core is a bad pointer." << std::endl;
      wyland_exit(-400);
    }

    std::cout << "[i]: initializing object 0x" << std::hex << reinterpret_cast<uintptr_t>(core) << std::endl;

    core->init(
      SYSTEM_SEGMENT_START, SYSTEM_SEGMENT_START+SYSTEM_SEGMENT_SIZE, 
      true, 0, &cache::linked_funcs, SYSTEM_SEGMENT_START, cache::GraphicsModulePtr, 
      cache::MMIOModule1Ptr, cache::DiskModulePtr, cache::MMIOModule2Ptr, cache::BiosPtr
    );

    run_core(core);

    delete core;
  }
};


TaskHandle make_disk = [](std::vector<std::string> &args) {
  if (args.size() == 0) {
    std::cerr << "[e]: " << std::invalid_argument("Excepted disk name.").what() << std::endl;
    wyland_exit(-1);
  }
  
  std::ofstream disk(args[0]);
  
  if (!disk) {
    std::cerr << "[e]: Unable to create file " << args[0] << std::endl;
    wyland_exit(-1);
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
    if (args[i].starts_with("-")) { // So files that starts with '-' aren't parsed... NOT MY PROBLEM AHAHAHA
      i++;
      continue;
    }
    
    std::ifstream file(args[i]);
    if (!file) {
      std::cerr << "[e]: Unable to create file " << args[i] << std::endl;
      wyland_exit(-1);
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

TaskHandle debug = [](std::vector<std::string> &args) {
  run_base_function(args, true);
};

TaskHandle libsof = [](std::vector<std::string> &args) {
  std::cout << "libsof():" << std::endl;
  for (const auto &file:args) {
    std::fstream disk(file);
    
    if (!disk) {
      std::cerr << "[e]: Unable to open disk file: " << file << std::endl;
      wyland_exit(-1);
    }
    
    wblock *block = new wblock;
    disk.read((char*)block->array, sizeof(block->array));
    auto header = wyland_files_make_header(block);
    delete block;
    
    task.target = header.target; /* To don't have target warning */
    
    if (!wyland_files_parse(&header, task.target, task.version)) {
      std::cerr << "[e]: " << std::invalid_argument("Invalid header file.").what() << std::endl;
      std::cout << "Extracted header:\n" << wyland_files_header_fmt(&header) << std::endl;
      wyland_exit(-1);
    }
    
    auto libs = get_libnames(disk, header.data, header, true);
    for (const auto&lib:libs) {
      std::cout << "from: " << lib.path << std::endl;
      for (const auto&func:lib.funcs) {
        std::cout << "\t(" << func.first << ")" << func.second << std::endl;
      }
    }
  }
};

TaskHandle check = [](std::vector<std::string> &args) {
  for (size_t i = 0; i < args.size(); i++) {
    if (args[i] == "--raw") {
      std::ifstream disk(args[++i]);
      wblock *block = new wblock;

      disk.read((char*)block->array, sizeof(block->array));

      for (unsigned int i = 0; i < sizeof(block->array); i++) std::cout << (int)block->array[i];
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

    if (args[i] == "--libs") {
      std::vector<std::string> libs_args{args[++i]};
      libsof(libs_args);
    }

    std::ifstream disk(args[i]);

    if (!disk) std::cerr << "[e]: not a file: " << args[i] << std::endl;

    wblock *block = new wblock;
    disk.read((char*)block->array, sizeof(block->array));
    auto header = wyland_files_make_header(block);
    
    std::cout << wyland_files_header_fmt(&header) << std::endl;
    // That's why i shouldn't use dynamic-pointers...
    delete block;
  }
};

TaskHandle t_update = [](std::vector<std::string> &) {
  std::cout << "[i]: updating Wyland..." << std::endl;
  clear_ressources(); // if there was an execution before 
  update(os.file_at_execution_path("updates"), os.file_at_execution_path("update_file"));
};

TaskHandle compile = [](std::vector<std::string> &args) {
  wysm::compile(args);
};

std::unordered_map<std::string, TaskHandle> handles {
  {"--v", version},
  {"--version", version},
  {"--n", name},
  {"--name", name},
  {"--b", build},
  {"--build", build},
  {"--target", target},
  {"--arch", target},
  {"--target-info", target_info},
  {"--info", infos}, 
  {"--i", infos},
  {"-check", check},
  {"-look", check},
  {"-target", set_target},
  {"-run", run},
  {"-run-raw", run_raw},
  {"-make-disk", make_disk},
  {"-build-disk", make_disk}, 
  {"-debug", debug},
  //{"-parse", parse},
  //{"-new-env", new_env},
  // Future tasks (Maybe in the std:wy4 standard !) (absolutely not..)
  {"-compile", compile},
  {"-c", compile},
  {"-libsof", libsof},
  {"-update", t_update},
  //{"-api", api}
};

static int WylandMain(int argc, char *const argv[]) {
  std::set_terminate(wyland_terminate);

  if (argc - 1 == 0) {
    std::cerr << "[e]: Expected a task." << std::endl;
    return -1;
  }

  std::cout << "========= Wyland " WYLAND_VERSION " : " WYLAND_BUILD_NAME " =========" << std::endl;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (handles.find(arg) != handles.end()) {
      if (arg.starts_with("--")) {
        std::vector<std::string> args{};
        handles[arg]({args});
        continue;
      }

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

//#ifndef ___WYLAND_STD_MAIN___
#define ___WYLAND_STD_MAIN___

int main(int argc, char *const argv[]) {
#ifdef _WIN32
  std::cout << "[w]: running Wyland on Windows, some features can be cursed. (Why still running code on Windows..?)" << std::endl;
#endif // _WIN32 ?
  return wylma::wyland::WylandMain(argc, argv);
}

//#endif // ___WYLAND_STD_MAIN___

/* Arguments: 
  --v, --version:  prints the version
  --n, --name:     prints the name
  --b, --build:    prints the build name
  --target:        prints the default target
  --target-info:   prints information about targets
  --check:         finds all files in .disk in ./ and ../        

  -target <x>:     sets the target to x
  -run <file>:     run <file> 
  -parse <file>:   parses the <file>.
  -debug <file>:   debugs a <file>
  -new-env <name>: creates a new environment

  --- in the future --- 
  -compile <file>: compiles the <file>
  -libsof <file>:  returns libs in <file>
  -api <lib>:      tries to load <lib> as library

  --- targets ---
  warch64: Basic Target 
  wtarg32: Basic Target in 32 bits mode
  wtargmarch: Target built for Mathematics Operations, and big floats.
  wtargfast: NON-STANDARD ! The fastest (jumps are limited to: jmp, je, jg and jl)

  Pay attention ! 
  -run <disk>, here, <disk> is not an input file like an OS. It's a disk !
  In a disk, you can find multiple informations, 
  such as the target (or targets, and adress of each target's code), 
  version, libs and important virtual addresses.
*/