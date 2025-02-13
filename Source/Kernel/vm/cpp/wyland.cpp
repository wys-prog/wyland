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
#include "wstream.hpp"

namespace stdfs = std::filesystem;

namespace wyland {
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
    systemfile,   // Files in .wyls
  };

  std::unordered_map<std::string, wobject_type> extentions2type = {
    {"res", wobject_type::res}, 
    {"log", wobject_type::logfile}, 
    {"cache", wobject_type::cache}, 
    {"vm", wobject_type::vm}, 
    {"bin", wobject_type::binfile}, 
    {"fnlib", wobject_type::functionlib}, 
    {"wyls", wobject_type::systemfile}, 
  };

  class wobject {
  private:
    std::fstream stream;

  public:
    std::string  path;
    wobject_type type;
    std::unordered_map<std::string, std::string> properties;

    void writeself() {
      if (!is_open()) open();
      writeline("type:" + std::to_string(int(type)));
      for (const auto &propertie : properties) 
        writeline(propertie.first + ":" + propertie.second);
    }

    void loadself() {
      while (!eof()) {
        std::string line = readline();
        size_t endname = line.find(':');
        if (endname == std::string::npos) {
          out.error("Syntax error:\t", line, ". Expected ':' token.");
          return;
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
    wobject(const stdfs::path &_p) : path(_p) {}
    wobject(const stdfs::path &_p, wobject_type _t) : path(_p), type(_t) {}
  };

  class VMHandle {
  private:
    kokuyoVM vm;
    wobject obj;
  public:
    VMHandle() = default;
    
    VMHandle(const std::string &name, stdfs::path path, log_level loglvl)  {
      obj.properties["name"] = "'" + name + "'";
      obj.properties["log-type"] = std::to_string(int(loglvl));
      obj.type = wobject_type::vm;
      // save the file
      obj.writeself();
    }
  };

  class Wyland {
  private:
    stdfs::path workspace;
    std::unordered_map<std::string, wobject&> objects;
    std::unordered_map<std::string, VMHandle> handles;

    void init() {
      if (!stdfs::exists(".wyland")) {
        std::cout << "Initializing..." << std::endl;
        for (auto &p : {
          ".wyland/", ".wyland/res/", ".wyland/libs/", 
          ".wyland/vm/", ".wyland/logs/", ".wyland/cache/", 
          ".wyland/shell/", ".wyland/shell/bin", ".wyland/shell/templates/", 
          ".wyland/logs/wyland.log",
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
      for (const auto &object : objects) 
        object.second.loadself();
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
    }

    int WylandExit = 0;
  };
} // wyland

int main(int argc, char *const argv[]) {
  std::cout << "Wyland —— 1.1" << std::endl;
  wyland::Wyland land(argc, argv);

  return land.WylandExit;
}