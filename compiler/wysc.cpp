#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <iostream>
#include <iomanip>

struct Alias {
  std::unordered_map<std::string, std::string> members;
};

struct Struct {
  Alias copy;
  std::string Tname;
};

struct Aliases {
  std::unordered_map<std::string, Alias> types;
  std::unordered_map<std::string, std::string> names;
};

struct SymbolTable {
  std::unordered_map<std::string, uint64_t> symbols;
  std::unordered_map<std::string, std::string> private_symbols;

  bool has_symbol(const std::string &name) {
    if (symbols.find(name) != symbols.end()) return true;
    else if (private_symbols.find(name) != private_symbols.end()) return true;
    return false;
  }
};

struct Globals {
  bool export_label_names;
};

class Compiler {
private:
  std::ifstream &input;
  std::ofstream &output;

  std::string line;
  size_t      line_count = 0;
  
  template <typename T>
  static std::vector<uint8_t> binof(T value) {
    std::vector<uint8_t> result(sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i) {
      result[sizeof(T) - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
    }
    return result;
  }
  
  static std::string trim(const std::string &what) {
    size_t start = what.find_first_not_of(" \t\n\r");
    size_t end = what.find_last_not_of(" \t\n\r");
    return (start == std::string::npos || end == std::string::npos)
         ? ""
         : what.substr(start, end - start + 1);
  }
  
  static std::string random_string(size_t length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
      result += characters[rand() % characters.size()];
    }

    return result;
  }

  static bool good_name(const std::string &str) {
    static const std::string illegals = "\n\t;/,=+*`£%-(){}[]\'\"&#<>";
    for (const auto &c:str) if (illegals.find(c) != std::string::npos) return false;
    return true;
  }

  int errors = 0;
  void generate_error(const std::string &what, const std::string &word) {
    std::cerr << "error: " << what << "\n\t| " << line_count << ":" << line << "\n\t|  ";
    for (size_t i = 0; i < std::to_string(line_count).size(); i++) std::cout << ' ';
    size_t beg = line.find(word);
    if (beg == std::string::npos) beg = 0;
    for (size_t i = 0; i < beg; i++) std::cout << ' ';
    for (const auto &c: word) std::cout << '~';
    std::cout << std::endl;
    errors++;
  }

  uint64_t current_address = 0;
  SymbolTable symbolTable;
  Aliases aliases;
  Globals globals;

  void compile_symbol(std::istringstream &iss, bool is_private) {
    std::string line = iss.str();
    size_t beg = line.find('"'), end = line.find('"', beg + 1);
    if (beg == std::string::npos || end == std::string::npos) { generate_error("excepted \" char after 'public' statement.", line); return; }
    std::string name = "";

    if (is_private) {
      name = random_string(4) + name + random_string(4);
    } else name = line.substr(beg + 1, end - (beg + 1));

    if (symbolTable.has_symbol(name)) {
      generate_error("re-definition of `" + name + "`...", name);
      return;
    }

    symbolTable.symbols[name] = current_address;
    if (globals.export_label_names) {
      output << ".jmp qword(" << current_address + name.size() + 1 << ")\n"
      ".array [string(\"" << name << "\"), byte(0x00)] ; function" << std::endl;
      std::cout << std::hex << "[i]: 0x" << std::setw(16) << std::setfill('0') << current_address << ": " << name << std::endl;
      current_address += name.size() + 1 + 9;
    }
    iss.str(line.substr(end + 1));
  }

  void compile_private(std::istringstream &iss) { compile_symbol(iss, true); }
  void compile_public(std::istringstream &iss) { compile_symbol(iss, false); }

  void compile_let(std::istringstream &iss) {
    std::string name, type;
    iss >> name >> type;

    if (!good_name(name)) {
      generate_error("Name uses illegal chars...", name);
      return;
    } else if (symbolTable.has_symbol(name)) {
      generate_error("re-definition of `" + name + "`...", name);
      return;
    }

    std::string privname = random_string(4) + name + random_string(4);
    symbolTable.private_symbols[name] = privname;
    symbolTable.symbols[privname] = current_address;

    if (type == "export") {
      output << ".jump qword(" << current_address + privname.size() + 1 << ")\n" 
      ".array [string(" << privname.c_str() << "), byte(0x00)]" << std::endl;;
      std::cout << "[i]: exported label `" << privname << "` at " << current_address << std::endl;
      current_address += privname.size() + 1 + 9 /*Jump + QWORD(x) + \0 at the end of string.*/;
      iss >> type;
    }

    std::string value = iss.str();
    output << type << "(" << trim(value) << ")" << std::endl;
    // "clear" the istringstream.
    iss.str("");
  }

  void declare_struct(std::istringstream &iss) {
    std::string name;
    iss >> name;

    Alias alias;

    if (trim(iss.str()) != "") {
      generate_error("extra-arguments after 'struct' declaration.", trim(iss.str()));
      return;
    }

    bool ended = false;
    while (std::getline(input, line) && !ended) {
      std::string Mtype, Mname; // Member name, Member type
      iss >> Mtype >> Mname;
      if (Mtype.empty()) continue;
      if (Mtype == "end") {ended = true; break;}

      if (Mname.empty()) {
        generate_error("excepted name", line);
        return;
      }

      if (aliases.names.find(Mtype) == aliases.names.end()) {
        generate_error("unknown type `" + Mtype + "`...", Mtype);
      }

      alias.members[Mname] = Mtype;
    }

    std::string Tname = random_string(4) + name + random_string(4);
    aliases.names[name] = Tname;
    aliases.types[Tname] = alias;
  }


public:
};

/*
public "label" 
  let i dword 0
  let msg array ["Hello, world !"]
  compute (90 + 78 - 0xFF * 78)
  
  if i equals 90
  elif i lesser 90
  else 
  end

  while false 
  end

  struct __int

  end

end 

*/