#define ___WYLAND_STD_MAIN___
#ifndef ___WYLAND_STD_MAIN___
#define ___WYLAND_STD_MAIN___

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <cstdlib>
#include <iomanip>

std::string buildCommandIO(const std::string &program,
                           const std::string &stdoutTarget = "", 
                           const std::string &stderrTarget = "", 
                           const std::string &stdinSource = "") {
  std::ostringstream cmd;
  cmd << program;

  if (!stdoutTarget.empty()) {
    cmd << " > " << stdoutTarget;
  }

  if (!stderrTarget.empty())  {
    cmd << " 2> " << stderrTarget;
  }

  if (!stdinSource.empty())  {
    cmd << " < " << stdinSource;
  }

  return cmd.str();
}

class property {
public:
  typedef struct {
    std::string type; /* external -> from the web ! internal -> local ! */
    std::string path;
    std::string name;
  } dependence;

  
private:
  static std::string quote(const std::string &what) { return "\"" + what + "\""; }

  std::string executable;
  std::string environnement;
  std::vector<std::string> arguments;
  std::string wstdout;
  std::string wstderr;
  std::string wstdin;
  std::string disk;
  std::string m1;
  std::string m2;
  std::string graphics_module;
  std::string curl_path;
  std::vector<dependence> dependencies;
  
  bool download_file(const std::string &link, const std::string &name) {
    std::stringstream cmd;
    cmd << curl_path << quote(link) << ' ' << quote(name);
    return (std::system(cmd.str().c_str()) == 0);
  }

  bool copy_file(const std::string &src_path, const std::string &dst_path) {
    return std::filesystem::copy_file(src_path, dst_path);
  }

  void get_dependencies() {
    for (size_t i = 0; i < dependencies.size(); i++) {
      auto dep = dependencies[i];
      std::cout << "[" << std::setw(dependencies.size()) << std::setfill(' ')
                << i + 1 << "/" << dependencies.size() << "]: getting dependency `" << dep.name << "` : " << std::flush;

      if (dep.type == "extern") {  
        std::cout << (download_file(dep.path, dep.name) ? "OK" : "FAILED") << std::endl;
      } else if (dep.type == "local") {
        std::cout << (copy_file(dep.path, dep.name) ? "OK" : "FAILED") << std::endl;
      } else {
        std::cout << "FAILED" << std::endl;
      }

      float percentage = ((i + 1) / static_cast<float>(dependencies.size())) * 100;
      std::cout << "[i]: progress: " << percentage << "%\r" << std::flush;
    }

    std::cout << std::endl;
  }

  int errors = 0;
  void generate_error(const std::string &what, const std::string &line, size_t line_count, const std::string &word) {
    std::cerr << "[compiler]: error: " << what << "\n\t| " << line_count << ":" << line << "\n\t|   ";
    size_t beg = line.find(word);
    if (beg == std::string::npos) beg = 0;
    for (size_t i = 0; i < beg; i++) std::cout << ' ';
    for (const auto &c: word) std::cout << '~';
    std::cout << std::endl;
    errors++;
  }

public:
  int open(const std::string &path) {
    if (!std::filesystem::exists(path)) {
      std::cerr << "[e]: no such file: " << path << std::endl;
      return -1;
    }

    std::ifstream input(path);
    std::string line;
    size_t line_count = 0;
    while (std::getline(input, line, ';')) {
      line_count++;
      size_t pos = line.find(':');
      if (pos == std::string::npos) {
        continue;
      }

      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      if (key == "executable") {
        executable = value;
      } else if (key == "arguments") {
        std::istringstream argStream(value);
        std::string arg;

        while (std::getline(argStream, arg, ',')) {
          arguments.push_back(arg);
        }

      } else if (key == "wstdout") {
        wstdout = value;
      } else if (key == "wstdin") {
        wstdin = value;
      } else if (key == "disk") {
        disk = value;
      } else if (key == "m1") {
        m1 = value;
      } else if (key == "m2") {
        m2 = value;
      } else if (key == "graphics_module") {
        graphics_module = value;
      } else if (key == "curl_path") {
        curl_path = value;
      } else if (key == "dependency") {
        std::istringstream depStream(value);
        dependence dep;
        std::getline(depStream, dep.type, ',');
        std::getline(depStream, dep.path, ',');
        std::getline(depStream, dep.name, ',');
        dependencies.push_back(dep);
      } else {
        generate_error("unknown key", line, line_count, key);
      }
    }

    return errors;
  }

  int run() {
    get_dependencies();
    std::ostringstream cmd;
    cmd << quote(executable) << ' ';
    for (const auto&arg:arguments) cmd << quote(arg) << ' ';
    cmd << quote(disk) << " -m1 " << quote(m1) << " -m2 " << quote(m2) << " -gm " << quote(graphics_module);
    return std::system(buildCommandIO(cmd.str(), wstdout, wstderr, wstdin).c_str());
  }
};

// TODO
// Exit code table

int main(/*int argc, char *const argv[]*/) {
  /*if (argc > 1)  {
    // TODO
  }*/

  property prop;
  if (prop.open("./") != 0) return -1;
  if (prop.run() != 0) return -1; // TODO !

  return 0;
}

#endif // ? ___WYLAND_STD_MAIN___