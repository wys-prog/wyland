#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <functional>
#include <filesystem>

struct Alias {
  std::unordered_map<std::string, std::string> members;
  size_t size;
  bool is_primary;
};

struct Struct {
  Alias copy;
  std::string Tname;
};

struct Aliases {
  std::unordered_map<std::string, Alias> types;
  std::unordered_map<std::string, std::string> names;

  bool is_declared(const std::string &string) {
    if (names.find(string) == names.end() || types.find(string) == types.end()) {
      return false;
    } return true;
  }
};

struct SymbolTable {
  std::unordered_map<std::string, uint64_t> symbols;
  std::unordered_map<std::string, std::string> private_symbols;

  bool has_symbol(const std::string &name) {
    if (symbols.find(name) != symbols.end()) return true;
    else if (private_symbols.find(name) != private_symbols.end()) return true;
    return false;
  }

  std::string get_symbol(const std::string &name) {
    if (private_symbols.find(name) != private_symbols.end()) {
      return private_symbols[name];
    } return name;
  }
};

struct Globals {
  bool export_label_names;
  bool export_symbol_table;
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
    if (beg == std::string::npos || word.empty()) beg = 0; // Don't trim word ! '  ' can be an error (but idk where)
    for (size_t i = 0; i < beg; i++) std::cout << ' ';
    for (const auto &c: word) std::cout << '~';
    std::cout << std::endl;
    errors++;
  }

  uint64_t current_address = 0;
  SymbolTable symbolTable {
    .symbols {}
  };

  Aliases aliases {
    .types {
      {"byte",  Alias{.members{{"x", "byte"}},  .size = 1, .is_primary = true}},
      {"word",  Alias{.members{{"x", "word"}},  .size = 2, .is_primary = true}},
      {"dword", Alias{.members{{"x", "dword"}}, .size = 4, .is_primary = true}},
      {"qword", Alias{.members{{"x", "qword"}}, .size = 8, .is_primary = true}},
    },
    .names {
      {"byte", "byte"},
      {"word", "word"},
      {"dword", "dword"},
      {"qword", "qword"},
    }
  };
  Globals &globals;

  std::vector<std::string> flatten_type(const std::string &type_name) {
    std::vector<std::string> result;
  
    if (aliases.types.find(type_name) == aliases.types.end()) {
      std::cerr << "flatten_type: unknown type `" << type_name << "`" << std::endl;
      return result;
    }
  
    const Alias &alias = aliases.types[type_name];
  
    if (alias.is_primary) {
      result.push_back(type_name);
      return result;
    }

    for (const auto &[member_name, member_type] : alias.members) {
      auto sub = flatten_type(member_type);
      result.insert(result.end(), sub.begin(), sub.end());
    }
  
    return result;
  }
  
  void compile_symbol(std::istringstream &iss, bool is_private) {
    std::string line = iss.str();
    size_t beg = line.find('"'), end = line.find('"', beg + 1);
    if (beg == std::string::npos || end == std::string::npos) { generate_error("excepted \" char after 'public' statement.", line); return; }
    std::string name = line.substr(beg + 1, end - (beg + 1));
    std::string shield_name = "";
    
    if (symbolTable.has_symbol(name)) {
      generate_error("re-definition of `" + name + "`...", name);
      return;
    }

    if (is_private) {
      shield_name = random_string(4) + name + random_string(4);
      symbolTable.private_symbols[name] = shield_name;
    } else shield_name = name;


    symbolTable.symbols[shield_name] = current_address;
    if (globals.export_label_names) {
      output << ".jmp qword(" << current_address + name.size() + 1 << ")\n"
      ".array [string(\"" << name << "\"), byte(0x00)] ; function" << std::endl;
      current_address += name.size() + 1 + 9;
    }
    std::cout << std::hex << "[i]: 0x" << std::setw(16) << std::setfill('0') << current_address << ": " << name << std::endl;
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

    if (!aliases.is_declared(type)) {
      generate_error("unknown type `" + type + "`", type);
      return;
    }
  }

  void declare_struct(std::istringstream &iss) {
    std::string name;
    iss >> name;

    if (aliases.names.find(name) != aliases.names.end()) {
      generate_error("redefinition of `" + name + "` struct...", name);
      return;
    } if (name.empty() || !good_name(name)) {
      generate_error("invalid name `" + name + "`", name);
      return;
    }

    Alias alias {.is_primary = false};

    /*if (trim(iss.str()) != "") {
      generate_error("extra-arguments after 'struct' declaration.", trim(iss.str()));
      return;
    }*/

    bool ended = false;
    while (std::getline(input, line)) {
      line_count++;
      std::string Mtype, Mname; // Member name, Member type
      std::istringstream iss(line); // Reset iss with the new line content
      iss >> Mtype >> Mname;
      if (Mtype.empty()) continue;
      if (Mtype == "end") {ended = true; break;}

      if (Mname.empty()) {
        generate_error("excepted name", line);
        return;
      }

      if (aliases.names.find(Mtype) == aliases.names.end()) {
        generate_error("unknown type `" + Mtype + "`...", Mtype);
        return;
      }

      alias.members[Mname] = Mtype;
      alias.size += aliases.types[Mtype].size;
    }

    if (!ended) {
      generate_error("unended struct `" + name + "`...", line);
      return;
    }

    std::string Tname = random_string(4) + name + random_string(4);
    aliases.names[name] = Tname;
    aliases.types[Tname] = alias;
    // Just to keep track on structures, we "export" it.
    output << "; struct `" << Tname << "` declared, template not exported." << std::endl;
  }

  void construct_struct(std::istringstream &iss) {
    std::string struct_name, name;
    iss >> struct_name >> name;
  
    if (aliases.names.find(struct_name) == aliases.names.end()) {
      generate_error("unknown type `" + struct_name + "`", struct_name);
      return;
    }
  
    if (symbolTable.has_symbol(name)) {
      generate_error("redefinition of `" + name + "`...", name);
      return;
    }
  
    std::string real = aliases.names[struct_name];
    Alias &type = aliases.types[real];
    
    output << "; struct instance: " << real << "\n.array [";

    auto types_to_emit = flatten_type(real);
    for (size_t i = 0; i < types_to_emit.size(); ++i) {
      if (i > 0) output << ", ";
      output << types_to_emit[i] << "(0x00)";
    }
    
    output << "]" << std::endl;
    
    current_address += type.size;
  }
  
  void compile_call(std::istringstream &iss) {
    std::string lined = iss.str();
    size_t beg = lined.find('"');
    size_t end = lined.find('"', beg + 1);

    if (beg == std::string::npos || end == std::string::npos) {
      generate_error("excepted quoted name.", lined);
      return;
    }

    std::string function = lined.substr(beg + 1, end - (beg + 1));

    if (function.empty() || !symbolTable.has_symbol(function)) {
      generate_error("cannot call `" + function + "` --It's not a function", function);
      return;
    }

    function = symbolTable.get_symbol(function);

    output << ".lqword byte(63), qword(" << current_address + 20 << ")\n"
              ".jmp qword(" << symbolTable.symbols[function] << ") ; function call: " << function 
              << std::endl;
    
    current_address += 20;
    iss.str(lined.substr(end + 1));
  }

  std::unordered_map<std::string, std::function<void(std::istringstream &)>> funcs {};

  void init() {
    funcs["call"] = [this](std::istringstream &iss){compile_call(iss);};
    funcs["private"] = [this](std::istringstream &iss){compile_private(iss);};
    funcs["public"] = [this](std::istringstream &iss){compile_public(iss);};
    funcs["let"] = [this](std::istringstream &iss){compile_let(iss);};
    funcs["struct"] = [this](std::istringstream &iss){declare_struct(iss);};
    funcs["construct"] = [this](std::istringstream &iss){construct_struct(iss);};
  }

public:
  Compiler(std::ifstream &in, std::ofstream &out, Globals &glob) : input(in), output(out), globals(glob) {
    init();
  }

  void compile() {
    while (std::getline(input, line)) {
      line_count++;
      std::istringstream iss(line);
      std::string word;
      while (iss >> word) {
        if (funcs.find(word) != funcs.end()) {
          funcs[word](iss);
        } else {
          generate_error("unknown token", word);
          return;
        }
      }
    }

    if (globals.export_symbol_table) {
      output << "; symbol table\n.array [string(\"symbol table\"), ";
      for (const auto &symbol : symbolTable.symbols) {
        output << "string(\"" << symbol.first << "\"), qword("
               << symbol.second << "), ";
      }
      output << "]\n";
    }
  }

  int get_errors() {
    return errors;
  }

};

int main(int argc, char *const argv[]) {
  srand(static_cast<unsigned int>(
    std::chrono::high_resolution_clock::now().time_since_epoch().count()));
  if (argc < 2) {
    std::cerr << "error: excepted input file." << std::endl;
    return (-'E'); // It's just for fun
  }
  
  std::string input;
  std::string output;
  Globals glob {
    .export_label_names = false, 
    .export_symbol_table = true,
  };

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "-export-label-names:no") { glob.export_label_names = false; }
    else if (std::string(argv[i]) == "-export-label-names:yes") { glob.export_label_names = true; }
    else if (std::string(argv[i]) == "-export-symbol-table:no") { glob.export_symbol_table = false; }
    else if (std::string(argv[i]) == "-export-symbol-table:yes") { glob.export_symbol_table = true; }
    else if (std::string(argv[i]) == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "excepted argument after '-o'." << std::endl;
        return (-'E');
      } output = (argv[++i]);
    } else {
      input = (argv[i]);
    }
  }

  std::ifstream ifile(input);
  std::ofstream ofile(output);

  Compiler compiler(ifile, ofile, glob);
  compiler.compile();
  
  return compiler.get_errors();
}

/*
public "label" 
  let i dword
  let msg array
  compute (90 + 78 - 0xFF * 78)
  
  if i equals 90
  elif i lesser 90
  else 
  end

  while false 
  end

  # this code is just idk !
  struct __int
    qword __intA
    qword __intB
  end

end 

*/