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

namespace stdfs = std::filesystem;

namespace wyland {
  inline uint64_t timeu64() {
    return static_cast<uint64_t>(std::time(nullptr));
  }
  
  stream_writer out{{"std:out", &std::cout}, };

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
    std::string get_name() { return obj.properties["name"]; }
    log_level get_log_level() { return llvl; }

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
      out.info("Virtual Machine loaded...");
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
  };

  class Wyland {
  private:
    stdfs::path workspace;
    std::unordered_map<std::string, wobject> objects;
    std::unordered_map<std::string, VMHandle> handles;
    std::unordered_map<std::string, std::function<int(std::vector<std::string>)>> ftable;

    int call(const std::string &name, const std::vector<std::string> &argv) {
      if (ftable.find(name) == ftable.end()) {
        std::string cmd = build_command(name, argv);
        int exit_code = std::system(cmd.c_str());
        if (exit_code != 0) out.error("Command '", cmd, "' exited with exit code: ", exit_code);
        return exit_code;
      }

      return ftable[name](argv);
    }

    int execute(const std::string &cmd) {
      std::string cmdcpy = cmd;
      trim(cmdcpy);

      if (!cmdcpy.empty()) {
        auto argv = extract_args(cmd);
        std::string name = std::move(argv.front()); 
        argv.erase(argv.begin());

        return call(name, argv);
      } 

      return 0;
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
        std::cout << "Initializing..." << std::endl;
        for (auto &p : {
          ".wyland/", ".wyland/res/", ".wyland/libs/", 
          ".wyland/vm/", ".wyland/logs/", ".wyland/cache/", 
          ".wyland/shell/", ".wyland/shell/bin", ".wyland/shell/templates/", 
          ".wyland/logs/wyland.log", ".wyland/logs/shell.log", 
          ".wyland/templates/disk.bin",
         }) {
          stdfs::path path = stdfs::absolute(p);
          stdfs::create_directories(path);
          out.log("Created: ", path);
        }
      } else {
        std::cout << "Wyland already initialized." << std::endl;
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
      out.log("Loading ", objects.size(), " objects...");
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
      std::vector<VMHandle> loadedHandles;
      out.log("Loading ", argv.size(), " elements...");
      for (size_t i = 0; i < argv.size(); i++) {
        loadedHandles.push_back(VMHandle(argv[i]));
        out.log(i, "/", argv.size(), " loaded.\r");
      }
      #error VMHandle::load() musn't take arguments.
      return 0;
    }

  public:

    Wyland(int argc, char *const argv[]) {
      workspace = "./.wyland/";

      std::ofstream wyland_log;
      wyland_log.open(".wyland/logs/wyland.log");

      if (wyland_log.is_open()) {
        out.add_stream("std:log", wyland_log);
      } else std::cerr << "?: [e]: Failed to open log file." << std::endl;
      
      if (!stdfs::exists(workspace)) {
        out.error("Wyland is not initalized in this directory.");
        out.error(workspace, ": No such directory.");
        out.log("Aborting.");
        WylandExit = -1;
        return;
      }

      launch();
      load();
    }

    int WylandExit = 0;

    void shell() {
      while (true) {
        std::string line;
        std::cout << '[' << workspace << "]\n> " << std::flush;
        std::getline(std::cin, line);
        WylandExit += execute(line);
      }
    }
  };
} // wyland

int main(int argc, char *const argv[]) {
  std::cout << "Wyland —— 1.1" << std::endl;
  wyland::Wyland land(argc, argv);

  return land.WylandExit;
}