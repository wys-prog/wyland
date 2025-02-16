#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <ctime>

#include "kokuyo.hpp"
#include "dlapi.h"
#include "wstream.hpp"
#include "wargs.hpp"
#include "algo.hpp"
#include "wbin.hpp"

namespace wyland {
  namespace stdfs = std::filesystem;
  stream_writer out{{"std:out", &std::cout}, };

  inline uint64_t timeu64() {
    return static_cast<uint64_t>(std::time(nullptr));
  }
  
  enum class log_level {
    all,
    err, 
    onbuff_all,
    onbuff_err,
  };

  enum class wobject_type {
    res,          // Files in .res
    logfile,      // Files in .log
    cache,        // Files in .cache
    vm,           // Files in .vm
    binfile,      // Files in .bin
    functionlib,  // Files in .fnlib
    propeties,    // Files in .propeties
    systemfile,   // Files in .wyls
  };

  std::unordered_map<std::string, wobject_type> extentions2type = {
    {".res", wobject_type::res}, 
    {".log", wobject_type::logfile}, 
    {".cache", wobject_type::cache}, 
    {".vm", wobject_type::vm}, 
    {".bin", wobject_type::binfile}, 
    {".fnlib", wobject_type::functionlib}, 
    {".wyls", wobject_type::systemfile}, 
    {".properties", wobject_type::propeties},
  };

  class wobject {
  private:
    std::fstream stream;

  public:
    std::string  path;
    wobject_type type;
    std::unordered_map<std::string, std::string> properties;
    std::vector<std::string> flags;

    void writeself() {
      if (!is_open()) open();
      writeline("type:" + std::to_string(int(type)));
      for (const auto &property : properties) 
        writeline(property.first + ":" + property.second);
    }

    void loadself() {
      while (!eof()) {
        std::string line = readline();
        trim(line);

        if (line.empty()) continue;

        size_t endname = line.find(':');
        if (endname == std::string::npos) {
          out.error("Syntax error:\t", line, ". Expected ':' token.");
          flags.push_back(std::runtime_error("Invalid Syntax.").what());
        } else {
          auto pname = line.substr(0, endname);
          auto value = line.substr(endname + 1);
          if (properties.find(pname) != properties.end()) out.warn("Redefinition of propertie '", pname, "'.");
          properties[pname] = value;
        }
      }
    }

    bool is_open() { return stream.is_open(); }

    bool open() {
      stream.open(path);
      if (!stream.is_open()) {
        std::ofstream creator(path);
        creator.close();
        stream.open(path);
      }
      return stream.is_open();
    }

    std::string readline() {
      std::string line;
      std::getline(stream, line);
      return line;
    }

    void writeline(const std::string &line) {
      stream << line << std::endl;
    }

    const uint8_t *read(std::streamsize count) {
      uint8_t *buff = new uint8_t[count];
      stream.read((char*)(buff), count);
      return buff;
    }

    void write(const uint8_t *data, std::streamsize len) {
      stream.write((const char*)data, len);
    }

    bool eof() { return stream.eof(); }

    void close() { return stream.close(); }

    std::fstream &get_stream() { return stream; }

    wobject() = default;
    wobject(const stdfs::path &_p) : path(_p) { open(); }
    wobject(const stdfs::path &_p, wobject_type _t) : path(_p), type(_t) { open(); }
  };

  class VMHandle {
  private:
    kokuyoVM vm;
    wobject obj;
    wobject logfile;
    log_level llvl;
    std::string ID;
    std::vector<uint8_t> input;

    void check_up() {
      if (obj.properties.find("log-file") == obj.properties.end()) {
        out.warn("Missing 'log-file' propertie in ", obj.path, ". Creating a new log file.");
        logfile.path = ".wylma/vm/" + obj.properties["name"] + "/logs.log";
        logfile.open();
        obj.properties["log-file"] = logfile.path;
      } else {
        logfile.path = obj.properties["log-file"];
        logfile.open();
      }

      if (obj.properties.find("log-type") == obj.properties.end()) {
        out.warn("Missing 'log-level' properite in ", obj.path, ". Defaulting to 1 (err).");
        llvl = log_level::err;
      } else {
        try {
          llvl = log_level(std::stoi(obj.properties["log-level"]));
        } catch(const std::invalid_argument& e) {
          out.error("C++ Exception: ", e.what());
          llvl = log_level::err;
        } catch(const std::out_of_range& e) {
          out.error("C++ Exception: ", e.what());
          llvl = log_level::err;
        } catch(const std::exception& e) {
          out.error("C++ Exception: ", e.what());
          llvl = log_level::err;
        }
      }
    }

    void check_input() {
      if (obj.properties.find("input") == obj.properties.end()) {
        std::ifstream file(obj.properties["input"]);
        if (file) {
          out.info("Loading input from ", obj.properties["input"], "...");
          wbin::read(obj.properties["input"], input);
          return;
        } else out.warn("Failed to open input file: ", obj.properties["input"]);
      } else out.error("Missing 'input' property in ", obj.path, ". Cannot load input.");

      out.info("Loading default input...");
      wbin::read(".wyland/templates/disk.bin", input);
    }

  public:
    static bool is_vm(const stdfs::path &path) {
      wobject obj(path);
      if (obj.properties.find("type") == obj.properties.end()
       && obj.properties["type"] != std::to_string(int(wobject_type::vm))
      ) return false;

      return true;
    }

    std::string get_name() { return obj.properties["name"]; }
    log_level get_log_level() { return llvl; }
    std::string get_id() { return ID; }

    VMHandle() = default;
    
    VMHandle(const std::string &name, stdfs::path path, 
      log_level loglvl, stdfs::path _logfile) : llvl(loglvl) { // Create a VM
      obj.properties["name"] = name;
      obj.properties["log-type"] = std::to_string(int(loglvl));
      obj.properties["log-file"] = _logfile;
      obj.type = wobject_type::vm;

      logfile.path = _logfile;
      logfile.open();
      ID = name + std::to_string(timeu64());
    }

    VMHandle(const stdfs::path &path) { // Load a VM
      obj.path = path;
      obj.loadself();

      if (obj.properties.find("name") == obj.properties.end()) {
        out.error("Missing 'name' property in ", obj.path, ". Cannot load the virtual machine.");
        return;
      }

      check_up();

      ID = obj.properties["name"] + std::to_string(timeu64());
    }

    void close() {
      obj.writeself();
      obj.close();
    }

    void load() {
      logfile.close();
      std::ofstream os(logfile.path);
      out.add_stream(ID, os);
      out.info("Virtual Machine loaded.");
    }

    void invoke() {
      out.info("Invoking virtual machine ", ID, "...");
      vm.invoke(input);
    }

    void kill() {
      out.info("Killing virtual machine ", ID, "...");
      out.remove_stream(ID);
      out.info("Killing ", ID, "'s virtual machine handle...");
      obj.writeself();
      obj.close();
    }

    VMHandle &operator=(const VMHandle &other) {
      if (this != &other) {
        vm = other.vm;
        obj.path = other.obj.path;
        logfile.path = other.logfile.path;
        llvl = other.llvl;
        ID = other.ID;
        input = other.input;
        // Open files.
        obj.open();
        logfile.open();
      }

      return *this;
    }
  };

  class Wyland {
  private:
    stdfs::path workspace;
    stdfs::path cdir; // Current Directory
    std::ofstream wyland_log;
    std::unordered_map<std::string, wobject> objects;
    std::unordered_map<std::string, VMHandle> handles;
    std::unordered_map<std::string, std::function<int(std::vector<std::string>)>> ftable;
    std::unordered_map<std::string, std::function<void()>> exCallables; // Call them when the program exits.
  
    int call(const std::string &name, const std::vector<std::string> &argv) {
      if (ftable.find(name) == ftable.end()) {
        std::vector<std::string> args = argv;
        args.insert(args.begin(), name);
        return ftable["--e"](args);
      }

      return ftable[name](argv);
    }

    int add_file(const std::string &path) {
      try {
        stdfs::path myPath = stdfs::absolute(path);
        stdfs::path myDest = ".wyland/res/" + myPath.filename().string() + myPath.extension().string();
        out.log("Adding file ", myPath, "...");
        return (int)stdfs::copy_file(myPath, myDest);
      } catch(const std::exception& e) {
        out.error("C++ Exception caught: ", e.what());
      }

      return -1;
    }

    int add_files(const std::string &dir) {
      try {
        stdfs::copy(dir, ".wylma/res/" + dir);
      } catch (const std::exception& e) {
        out.error("C++ Exception caught: ", e.what());
        return -1;
      }
      return 0;
    }

    void init() {
      if (!stdfs::exists(".wyland")) {
        out.log("Initializing...");
        for (auto &p : {
          ".wyland/", ".wyland/res/", ".wyland/libs/", 
          ".wyland/vm/", ".wyland/logs/", ".wyland/cache/", 
          ".wyland/shell/", ".wyland/shell/bin", ".wyland/shell/templates/", 
          ".wyland/shell/configs/", ".wyland/res/system/", 
         }) {
          stdfs::path path = stdfs::absolute(p);
          stdfs::create_directories(path);
          out.log("Created: ", path);
        }
      } else {
        out.log("Wyland is already initialized in this directory.");
      }
    }

    void launch() {
      for (const auto &dir : stdfs::recursive_directory_iterator(workspace)) {
        if (dir.is_regular_file()) {
          stdfs::path path = dir.path();
          if (extentions2type.find(path.extension()) != extentions2type.end()) {
            objects[path.filename()] = wobject(path, extentions2type[path.extension()]);
            out.log("Added file: ", path);
          }
        }
      }
    }

    void load() {
      out.info("Loading ", objects.size(), " objects...");
      for (auto &object : objects) 
        object.second.loadself();
    }

    int add(const std::vector<std::string> &argv) {
      int err = 0;
      for (size_t i = 0; i < argv.size(); i++) {
        if (trimRT(argv[i]) == "--f") err += add_files(argv[++i]);
        else err += add_file(argv[i]);
      }

      return err;
    }

    int loadvm(const std::vector<std::string> &argv) {
      uint64_t count = 0;
      out.info("Loading ", argv.size(), " elements...");
      for (size_t i = 0; i < argv.size(); i++) {
        if (!VMHandle::is_vm(argv[i])) {
          out.error("Invalid VM file: ", argv[i]);
          continue;
        }
        // Load the virtual machine
        VMHandle handle(argv[i]);
        handles[handle.get_id()] = handle;
        out.log(i, "/", argv.size(), " loaded.\r");
      }

      out.info("Loaded ", count, " virtual machines.");

      return 0;
    }

    int unloadvm(const std::vector<std::string> &argv) {
      uint64_t count = 0;
      out.info("Unloading ", argv.size(), " elements...");
      for (size_t i = 0; i < argv.size(); i++) {
        if (handles.find(argv[i]) == handles.end()) {
          out.error("Invalid VM ID: ", argv[i]);
          continue;
        }
        // Unload the virtual machine
        handles[argv[i]].close();
        handles.erase(argv[i]);
        out.info(i, "/", argv.size(), " unloaded.\r");
      }

      return 0;
    }

    int invoke(const std::vector<std::string> &argv) {
      uint64_t count = 0;
      out.info("Invoking ", argv.size(), " elements...");
      for (size_t i = 0; i < argv.size(); i++) {
        if (handles.find(argv[i]) == handles.end()) {
          out.error("Invalid VM ID: ", argv[i]);
          continue;
        }
        // Invoke the virtual machine
        handles[argv[i]].invoke();
        out.info(i, "/", argv.size(), " invoked.\r");
      }

      return 0;
    }

    int kill(const std::vector<std::string> &argv) {
      uint64_t count = 0;
      out.log("Killing ", argv.size(), " elements...");
      for (size_t i = 0; i < argv.size(); i++) {
        if (handles.find(argv[i]) == handles.end()) {
          out.error("Invalid VM ID: ", argv[i]);
          continue;
        }
        // Kill the virtual machine
        handles[argv[i]].kill();
        out.log(i, "/", argv.size(), " killed.\r");
      }

      return 0;
    }

    int listvm(const std::vector<std::string> &argv) { // Lists all VMs (loaded )
      for (auto &VM : handles) 
        out.log("ID: ", VM.first, " name: ", VM.second.get_name());
      
      out.log("Total: ", handles.size(), " virtual machines loaded.");

      return 0;
    }

    int list(const std::vector<std::string> &argv) {
      for (auto &object : objects) {
        out.log(
          "———————————————"
          "Type: ", (int)object.second.type, 
          " Path: ", object.second.path, 
          " Properties: ", object.second.properties.size()
        );
      }

      return 0;
    }

    int shell(const std::vector<std::string> &argv) {
      if (!stdfs::exists(".wyland/shell")) {
        out.error("Shell is not initalized in this directory.");
        return -1;
      }

      std::string cmd;
      while (true) {
        std::cout << '[' << workspace << "]\n % ";
        std::getline(std::cin, cmd);
        execute(cmd);
      }

      return WylandExit;
    }

    int run(const std::vector<std::string> &argv) {
      if (argv.size() < 1) {
        out.error("Exepcted a path to a file.");
        return -1;
      }

      std::string path = argv[0];
      if (!VMHandle::is_vm(path)) {
        out.error("Invalid VM file: ", path);
        return -1;
      }

      VMHandle handle(path);
      
      handle.load();
      handle.invoke();

      return 0;
    }

    int create_vm(const std::vector<std::string> &argv) {
      if (argv.size() < 1) {
        out.error("Expected a name for the virtual machine.");
        return -1;
      }

      std::string name = argv[0];
      stdfs::path path = ".wyland/vm/" + name + ".vm";
      stdfs::path logpath = ".wyland/logs/" + name + ".log";
      VMHandle handle(name, path, log_level::err, logpath);

      handles[handle.get_id()] = handle;
      out.info("Created virtual machine: ", name, " with ID: ", handle.get_id(), ".");
      exCallables[handle.get_id()] = [&]() { handle.close(); };
      return 0;
    }

    int edit_properties(const std::vector<std::string> &argv) {
      // --edit <file>, [propertie, value, propertie, value, ...]
      if (argv.size() < 3) {
        out.error("Expected a file and propertie-value pairs.");
        return -1;
      }

      std::string path = argv[0];
      if (objects.find(path) == objects.end()) {
        out.error("Invalid file: ", path);
        return -1;
      }

      wobject &obj = objects[path];
      for (size_t i = 1; i < argv.size(); i += 2) {
        if (i + 1 >= argv.size()) {
          out.error("Expected a value for propertie: ", argv[i]);
          return -1;
        }

        obj.properties[argv[i]] = argv[i + 1];
      }

      exCallables[path] = [&]() { obj.writeself(); };
      return 0;
    }

    void loadtable() {
      ftable["--add"] = [this](std::vector<std::string> argv) { return add(argv); };
      ftable["--loadvm"] = [this](std::vector<std::string> argv) { return loadvm(argv); };
      ftable["--unloadvm"] = [this](std::vector<std::string> argv) { return unloadvm(argv); }; 
      ftable["--invoke"] = [this](std::vector<std::string> argv) { return invoke(argv); };
      ftable["--kill"] = [this](std::vector<std::string> argv) { return kill(argv); };
      ftable["--listvm"] = [this](std::vector<std::string> argv) { return listvm(argv); };
      ftable["--list"] = [this](std::vector<std::string> argv) { return list(argv); };
      ftable["--shell"] = [this](std::vector<std::string> argv) { return shell(argv); };
      ftable["--init"] = [this](std::vector<std::string> argv) { init(); return 0; };
      ftable["--exec"] = [this](std::vector<std::string> argv) { return execute(to_string(argv)); };
      ftable["--run"] = [this](std::vector<std::string> argv) { return run(argv); };
      ftable["--exit"] = [this](std::vector<std::string> argv) { exit(0); return 0; };
      ftable["exit"] = [this](std::vector<std::string> argv) { exit(0); return 0; };
      ftable["--e"] = [this](std::vector<std::string> argv) { return std::system(to_string(argv).c_str()); };
      ftable["--createvm"] = [this](std::vector<std::string> argv) { return create_vm(argv); };
      ftable["--edit"] = [this](std::vector<std::string> argv) { return edit_properties(argv); };
      ftable["--help"] = [this](std::vector<std::string> argv) {
        out.info("Wyland —— 1.1");
        out.info("Commands:");
        out.info("  --add <file>...: Add files to the workspace.");
        out.info("  --loadvm <file>...: Load virtual machines.");
        out.info("  --unloadvm <id>...: Unload virtual machines.");
        out.info("  --invoke <id>...: Invoke virtual machines.");
        out.info("  --kill <id>...: Kill virtual machines.");
        out.info("  --listvm: List all loaded virtual machines.");
        out.info("  --list: List all objects in the workspace.");
        out.info("  --shell: Launch the Wyland shell.");
        out.info("  --init: Initialize Wyland in the current directory.");
        out.info("  --exec <command>...: Execute a command.");
        out.info("  --run <file>: Run a virtual machine.");
        out.info("  --exit: Exit Wyland.");
        out.info("  exit: Exit Wyland.");
        out.info("  --e <command>...: Execute a system command.");
        out.info("  --createvm <name>: Create a virtual machine.");
        out.info("  --edit <file> <propertie> <value>...: Edit properties of a file.");
        return 0;
      };
    }

  public:

    Wyland() {
      loadtable();
      workspace = "./.wyland/";

      wyland_log.open(".wyland/logs/wyland.log");

      if (wyland_log.is_open()) {
        out.add_stream("std:log", wyland_log);
      } else std::cerr << "?: [e]: Failed to open log file." << std::endl;
      
      if (!stdfs::exists(workspace)) {
        out.error("Wyland is not initalized in this directory.");
        out.error(workspace, ": No such directory.");
        out.info("Use wyland --init to initialize Wyland in this directory.");
        return;
      }

      launch();
      load();
    }

    int execute(const std::string &cmd) {
      std::string cmdcpy = cmd;
      trim(cmdcpy);

      if (!cmdcpy.empty()) {
        auto argv = extract_args(cmd);
        std::string name = extract_first(argv);

        int ext = call(name, argv);
        if (ext != 0) {
          out.error("Failed to execute command: ", cmd, " with exit code: ", ext, ".");
          return ext;
        }
      } 

      return 0;
    }

    int WylandExit = 0;
  };

} // wyland

int main(int argc, char *const argv[]) {
  std::cout << "Wyland —— 1.1" << std::endl;
  
  try {
    wyland::Wyland land;

    for (int i = 1; i < argc; i++) {
      land.WylandExit += land.execute(argv[i]);
    }

    return land.WylandExit;
  } catch(const std::exception& e) {
    wyland::out.error("C++ Exception: ", e.what());
  } catch(...) {
    wyland::out.error("C++ Exception: Unknown");
  }
  
  return -1;
}

