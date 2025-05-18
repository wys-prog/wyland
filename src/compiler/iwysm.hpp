#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <filesystem>

#ifndef _WIN32
#include <unistd.h>
#endif // !? Win 

#include "../wmmbase.hpp"

namespace wylma {
  namespace wyland {
    namespace wysm {
      template <typename T>
      std::vector<uint8_t> wysm_binof(T value) {
        std::vector<uint8_t> result(sizeof(T));
        for (size_t i = 0; i < sizeof(T); ++i) {
          result[sizeof(T) - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
        }
        return result;
      }

      std::vector<uint8_t> wysm_binof(const std::string &value) {
        std::vector<uint8_t> result;
        
        for (const auto&c:value) result.push_back((uint8_t)c);

        return result;
      }

      std::string trim(const std::string &what) {
        size_t start = what.find_first_not_of(" \t\n\r");
        size_t end = what.find_last_not_of(" \t\n\r");
        return (start == std::string::npos || end == std::string::npos)
            ? ""
            : what.substr(start, end - start + 1);
      }

      int errors = 0;
      void generate_error(const std::string &what, const std::string &line, size_t line_count, const std::string &word) {
        std::cerr << "error: " << what << "\n\t| " << line_count << ":" << (line.empty() ? "<empty>" : line) << "\n\t|  ";
        for (size_t i = 0; i < std::to_string(line_count).size(); i++) std::cerr << ' ';
        size_t beg = line.find(word);
        if (beg == std::string::npos) beg = 0;
        for (size_t i = 0; i < beg; i++) std::cerr << ' ';
        for (size_t i = 0; i < word.size(); i++) std::cerr << '~';
        std::cerr << std::endl;
        errors++;
      }

      bool is_numeric_string(const std::string &string) {
        for (const auto&c:string) {
          if (!ishexnumber(c)) return false;
        }
        return true;
      }

      struct undefined_reference {
        uint64_t filepos;
        std::string line;
        uint64_t line_count;
        std::string refname;
      };

      class WylandAssembler {
        std::string Arch;
        uint64_t current_address = 0;
        std::unordered_map<std::string, std::pair<std::string, std::vector<uint8_t>>> instructions;
        std::unordered_map<std::string, uint64_t> symbols;
        std::unordered_map<std::string, std::string> macros;
        std::unordered_map<std::string, std::vector<undefined_reference>> unresolved_references;

        std::vector<uint8_t> parse_array(const std::string& raw, const std::string& line, size_t line_num) {
          std::vector<uint8_t> result;
        
          if (raw.front() != '[' || raw.back() != ']') {
            generate_error("Invalid array syntax, expected square brackets", line, line_num, raw);
            return {};
          }
        
          std::string inside = raw.substr(1, raw.size() - 2);
          std::stringstream ss(inside);
          std::string token;
        
          while (std::getline(ss, token, ',')) {
            token = trim(token);
            if (token.empty()) continue;
        
            size_t beg = token.find('(');
            size_t end = token.rfind(')');
        
            if (beg == std::string::npos || end == std::string::npos || beg >= end) {
              generate_error("Invalid array element syntax", line, line_num, token);
              return {};
            }
        
            std::string type = token.substr(0, beg);
            std::string val = token.substr(beg + 1, end - beg - 1);
        
            if (type == "byte") {
              uint8_t value = static_cast<uint8_t>(std::stoul(val, nullptr, 16));
              auto b = wysm_binof(value);
              result.insert(result.end(), b.begin(), b.end());
            } else if (type == "word") {
              uint16_t value = static_cast<uint16_t>(std::stoul(val, nullptr, 16));
              auto b = wysm_binof(value);
              result.insert(result.end(), b.begin(), b.end());
            } else if (type == "dword") {
              uint32_t value = static_cast<uint32_t>(std::stoul(val, nullptr, 16));
              auto b = wysm_binof(value);
              result.insert(result.end(), b.begin(), b.end());
            } else if (type == "qword") {
              uint64_t value = static_cast<uint64_t>(std::stoull(val, nullptr, 16));
              auto b = wysm_binof(value);
              result.insert(result.end(), b.begin(), b.end());
            } else if (type == "char") {
              if (val.size() == 3 && val.front() == '\'' && val.back() == '\'') {
                char c = val[1];
                result.push_back(static_cast<uint8_t>(c));
              } else {
                generate_error("Invalid char format. Expected 'X'", line, line_num, val);
                return {};
              }
            } else {
              generate_error("Unsupported array element type", line, line_num, type);
              return {};
            }
          }

          current_address += result.size();
        
          return result;
        }
        
        void init_table_x64() {
          // Define instruction format and opcode (1 byte)
          instructions[".nop"] = {"", {0x00}};
          instructions[".lea"] = {"byte, qword", {0x01}};
          /* Loads */
          instructions[".lbyte"] = {"byte, byte", {0x02, 1 * 8}};
          instructions[".lword"] = {"byte, word", {0x02, 2 * 8}};
          instructions[".ldword"] = {"byte, dword", {0x02, 4 * 8}};
          instructions[".lqword"] = {"byte, qword", {0x02, 8 * 8}};
          /* Stores */
          instructions[".store"] = {"byte, byte, qword", {3}};

          instructions[".mov"] = {"byte, byte", {4}};
          instructions[".add"] = {"byte, byte", {5}};
          instructions[".sub"] = {"byte, byte", {6}};
          instructions[".mul"] = {"byte, byte", {7}};
          instructions[".div"] = {"byte, byte", {8}};
          instructions[".mod"] = {"byte, byte", {9}};
          instructions[".jmp"] = {"qword", {10}};
          instructions[".je"] = {"qword", {11}};
          instructions[".jne"] = {"qword", {12}};
          instructions[".jl"] = {"qword", {13}};
          instructions[".jg"] = {"qword", {14}};
          instructions[".jle"] = {"qword", {15}};
          instructions[".jlg"] = {"qword", {16}};
          instructions[".cmp"] = {"byte, byte", {17}};
          instructions[".int"] = {"dword", {18}};
          instructions[".loadat"] = {"byte, qword", {19}};
          instructions[".ret"] = {"", {20}};
          instructions[".movad"] = {"byte, byte", {21}};
          instructions[".sal"] = {"byte, qword", {22}};
          instructions[".sar"] = {"byte, qword", {23}};
          instructions[".throw"] = {"", {24}};
          instructions[".clfn"] = {"dword", {25}};
          instructions[".call-c"] = {"dword", {25}};
          instructions[".emplace"] = {"qword, byte", {26}};
          instructions[".pushmmio"] = {"byte, byte", {27}};
          instructions[".popmmio"] = {"byte", {28}};
          instructions[".connectmmio"] = {"byte, qword", {29}};
          instructions[".deconnectmmio"] = {"byte, qword", {30}};
          instructions[".ctmmio"] = {"byte, qword", {29}};
          instructions[".dcmmio"] = {"byte, qword", {30}};
          instructions[".xor"] = {"byte, byte", {31}};
          instructions[".or"] = {"byte, byte", {32}};
          instructions[".and"] = {"byte, byte", {33}};
          instructions[".db"] = {"byte", {}};
          instructions[".dw"] = {"word", {}};
          instructions[".dd"] = {"dword", {}};
          instructions[".dq"] = {"qword", {}};
          instructions[".inc"] = {"byte", {34}};
          instructions[".dec"] = {"byte", {35}};
          instructions[".header"] = {"word, dword, qword, qword, qword", {0x77, 0x6C, 0x66}};
          // Define registers (idk why, some of them didn't work..)
          macros["%bmm0"] = "byte(0x00)";
          macros["%wmm0"] = "byte(0x10)";
          macros["%dmm0"] = "byte(0x20)";
          macros["%bmm1"] = "byte(0x01)";
          macros["%wmm1"] = "byte(0x11)";
          macros["%dmm1"] = "byte(0x21)";
          macros["%bmm2"] = "byte(0x02)";
          macros["%wmm2"] = "byte(0x12)";
          macros["%dmm2"] = "byte(0x22)";
          macros["%bmm3"] = "byte(0x03)";
          macros["%wmm3"] = "byte(0x13)";
          macros["%dmm3"] = "byte(0x23)";
          macros["%bmm4"] = "byte(0x04)";
          macros["%wmm4"] = "byte(0x14)";
          macros["%dmm4"] = "byte(0x24)";
          macros["%bmm5"] = "byte(0x05)";
          macros["%wmm5"] = "byte(0x15)";
          macros["%dmm5"] = "byte(0x25)";
          macros["%bmm6"] = "byte(0x06)";
          macros["%wmm6"] = "byte(0x16)";
          macros["%dmm6"] = "byte(0x26)";
          macros["%bmm7"] = "byte(0x07)";
          macros["%wmm7"] = "byte(0x17)";
          macros["%dmm7"] = "byte(0x27)";
          macros["%bmm8"] = "byte(0x08)";
          macros["%wmm8"] = "byte(0x18)";
          macros["%dmm8"] = "byte(0x28)";
          macros["%bmm9"] = "byte(0x09)";
          macros["%wmm9"] = "byte(0x19)";
          macros["%dmm9"] = "byte(0x29)";
          macros["%bmm10"] = "byte(0x0a)";
          macros["%wmm10"] = "byte(0x1a)";
          macros["%dmm10"] = "byte(0x2a)";
          macros["%bmm11"] = "byte(0x0b)";
          macros["%wmm11"] = "byte(0x1b)";
          macros["%dmm11"] = "byte(0x2b)";
          macros["%bmm12"] = "byte(0x0c)";
          macros["%wmm12"] = "byte(0x1c)";
          macros["%dmm12"] = "byte(0x2c)";
          macros["%bmm13"] = "byte(0x0d)";
          macros["%wmm13"] = "byte(0x1d)";
          macros["%dmm13"] = "byte(0x2d)";
          macros["%bmm14"] = "byte(0x0e)";
          macros["%wmm14"] = "byte(0x1e)";
          macros["%dmm14"] = "byte(0x2e)";
          macros["%bmm15"] = "byte(0x0f)";
          macros["%wmm15"] = "byte(0x1f)";
          macros["%dmm15"] = "byte(0x2f)";
          macros["%qmm0"] = "byte(0x30)";
          macros["%qmm1"] = "byte(0x31)";
          macros["%qmm2"] = "byte(0x32)";
          macros["%qmm3"] = "byte(0x33)";
          macros["%qmm4"] = "byte(0x34)";
          macros["%qmm5"] = "byte(0x35)";
          macros["%qmm6"] = "byte(0x36)";
          macros["%qmm7"] = "byte(0x37)";
          macros["%qmm8"] = "byte(0x38)";
          macros["%qmm9"] = "byte(0x39)";
          macros["%qmm10"] = "byte(0x3a)";
          macros["%qmm11"] = "byte(0x3b)";
          macros["%qmm12"] = "byte(0x3c)";
          macros["%qmm13"] = "byte(0x3d)";
          macros["%qmm14"] = "byte(0x3e)";
          macros["%qmm15"] = "byte(0x3f)";
          macros["%qmm16"] = "byte(0x40)";
          macros["%qmm17"] = "byte(0x41)";
          macros["%qmm18"] = "byte(0x42)";
          macros["%qmm19"] = "byte(0x43)";
          macros["%qmm20"] = "byte(0x44)";
          macros["%qmm21"] = "byte(0x45)";
          macros["%qmm22"] = "byte(0x46)";
          macros["%qmm23"] = "byte(0x47)";
          macros["%qmm24"] = "byte(0x48)";
          macros["%qmm25"] = "byte(0x49)";
          macros["%qmm26"] = "byte(0x4a)";
          macros["%qmm27"] = "byte(0x4b)";
          macros["%qmm28"] = "byte(0x4c)";
          macros["%qmm29"] = "byte(0x4d)";
          macros["%qmm30"] = "byte(0x4e)";
          macros["%qmm31"] = "byte(0x4f)";
        }

        void init_table_x128() {
          std::cout << "using x128 arch." << std::endl;
          // Instructions (voir switch dans warch128.hpp)
          instructions[".nop"] = {"", {0x00}};
          instructions[".lea"] = {"byte, qword", {0x01}};
          instructions[".mov"] = {"byte, byte, byte", {0x01}};
          instructions[".add"] = {"byte, byte, byte", {0x02}};
          instructions[".sub"] = {"byte, byte, byte", {0x03}};
          instructions[".mul"] = {"byte, byte, byte", {0x04}};
          instructions[".div"] = {"byte, byte, byte", {0x05}};
          instructions[".mod"] = {"byte, byte, byte", {0x06}};
          instructions[".cmp"] = {"byte, byte, byte", {0x07}};
          instructions[".jmp"] = {"qword", {0x08}};
          instructions[".je"] = {"qword", {0x09}};
          instructions[".jne"] = {"qword", {0x0A}};
          instructions[".jl"] = {"qword", {0x0B}};
          instructions[".jle"] = {"qword", {0x0C}};
          instructions[".jg"] = {"qword", {0x0D}};
          instructions[".jge"] = {"qword", {0x0E}};
          instructions[".ijmprgs"] = {"byte, byte", {0x0F}};
          instructions[".jergs"] = {"byte, byte", {0x10}};
          instructions[".jnergs"] = {"byte, byte", {0x11}};
          instructions[".jlregs"] = {"byte, byte", {0x12}};
          instructions[".jlergs"] = {"byte, byte", {0x13}};
          instructions[".jgrgs"] = {"byte, byte", {0x14}};
          instructions[".jgergs"] = {"byte, byte", {0x15}};
          instructions[".throw"] = {"", {0x16}};
          instructions[".int"] = {"dword", {0x17}};
          instructions[".movis"] = {"byte", {0x18}};
          instructions[".addis"] = {"byte", {0x19}};
          instructions[".subis"] = {"byte", {0x1A}};
          instructions[".mulis"] = {"byte", {0x1B}};
          instructions[".divis"] = {"byte", {0x1C}};
          instructions[".modis"] = {"byte", {0x1D}};
          instructions[".sal"] = {"byte, byte", {0x1E}};
          instructions[".sar"] = {"byte, byte", {0x1F}};
          instructions[".salis"] = {"byte", {0x20}};
          instructions[".saris"] = {"byte", {0x21}};
          instructions[".dec"] = {"byte, byte", {0x22}};
          instructions[".inc"] = {"byte, byte", {0x23}};
          instructions[".and"] = {"byte, byte, byte", {0x24}};
          instructions[".xor"] = {"byte, byte, byte", {0x25}};
          instructions[".or"] = {"byte, byte, byte", {0x26}};
          instructions[".andis"] = {"byte", {0x27}};
          instructions[".xoris"] = {"byte", {0x28}};
          instructions[".oris"] = {"byte", {0x29}};
          instructions[".call-c"] = {"dword", {0x2A}};
          instructions[".dbgfalg"] = {"", {0xFE}};
          instructions[".halt"] = {"", {0xFF}};

          instructions[".mov-byte"] = {"byte, byte", {0x01, 8}};
            // mov, add, sub, mul, div, mod, cmp, sal, sar, and, or, xor for all sizes
            instructions[".mov-byte"] = {"byte, byte", {0x01, 8}};
            instructions[".mov-word"] = {"byte, word", {0x01, 16}};
            instructions[".mov-dword"] = {"byte, dword", {0x01, 32}};
            instructions[".mov-qword"] = {"byte, qword", {0x01, 64}};
            instructions[".mov-dqword"] = {"byte, dqword", {0x01, 128}};

            instructions[".add-byte"] = {"byte, byte", {0x02, 8}};
            instructions[".add-word"] = {"byte, word", {0x02, 16}};
            instructions[".add-dword"] = {"byte, dword", {0x02, 32}};
            instructions[".add-qword"] = {"byte, qword", {0x02, 64}};
            instructions[".add-dqword"] = {"byte, dqword", {0x02, 128}};

            instructions[".sub-byte"] = {"byte, byte", {0x03, 8}};
            instructions[".sub-word"] = {"byte, word", {0x03, 16}};
            instructions[".sub-dword"] = {"byte, dword", {0x03, 32}};
            instructions[".sub-qword"] = {"byte, qword", {0x03, 64}};
            instructions[".sub-dqword"] = {"byte, dqword", {0x03, 128}};

            instructions[".mul-byte"] = {"byte, byte", {0x04, 8}};
            instructions[".mul-word"] = {"byte, word", {0x04, 16}};
            instructions[".mul-dword"] = {"byte, dword", {0x04, 32}};
            instructions[".mul-qword"] = {"byte, qword", {0x04, 64}};
            instructions[".mul-dqword"] = {"byte, dqword", {0x04, 128}};

            instructions[".div-byte"] = {"byte, byte", {0x05, 8}};
            instructions[".div-word"] = {"byte, word", {0x05, 16}};
            instructions[".div-dword"] = {"byte, dword", {0x05, 32}};
            instructions[".div-qword"] = {"byte, qword", {0x05, 64}};
            instructions[".div-dqword"] = {"byte, dqword", {0x05, 128}};

            instructions[".mod-byte"] = {"byte, byte", {0x06, 8}};
            instructions[".mod-word"] = {"byte, word", {0x06, 16}};
            instructions[".mod-dword"] = {"byte, dword", {0x06, 32}};
            instructions[".mod-qword"] = {"byte, qword", {0x06, 64}};
            instructions[".mod-dqword"] = {"byte, dqword", {0x06, 128}};

            instructions[".cmp-byte"] = {"byte, byte", {0x07, 8}};
            instructions[".cmp-word"] = {"byte, word", {0x07, 16}};
            instructions[".cmp-dword"] = {"byte, dword", {0x07, 32}};
            instructions[".cmp-qword"] = {"byte, qword", {0x07, 64}};
            instructions[".cmp-dqword"] = {"byte, dqword", {0x07, 128}};

            instructions[".sal-byte"] = {"byte, byte", {0x1E, 8}};
            instructions[".sal-word"] = {"byte, word", {0x1E, 16}};
            instructions[".sal-dword"] = {"byte, dword", {0x1E, 32}};
            instructions[".sal-qword"] = {"byte, qword", {0x1E, 64}};
            instructions[".sal-dqword"] = {"byte, dqword", {0x1E, 128}};

            instructions[".sar-byte"] = {"byte, byte", {0x1F, 8}};
            instructions[".sar-word"] = {"byte, word", {0x1F, 16}};
            instructions[".sar-dword"] = {"byte, dword", {0x1F, 32}};
            instructions[".sar-qword"] = {"byte, qword", {0x1F, 64}};
            instructions[".sar-dqword"] = {"byte, dqword", {0x1F, 128}};

            instructions[".and-byte"] = {"byte, byte, byte", {0x24, 8}};
            instructions[".and-word"] = {"byte, word, word", {0x24, 16}};
            instructions[".and-dword"] = {"byte, dword, dword", {0x24, 32}};
            instructions[".and-qword"] = {"byte, qword, qword", {0x24, 64}};
            instructions[".and-dqword"] = {"byte, dqword, dqword", {0x24, 128}};

            instructions[".or-byte"] = {"byte, byte, byte", {0x26, 8}};
            instructions[".or-word"] = {"byte, word, word", {0x26, 16}};
            instructions[".or-dword"] = {"byte, dword, dword", {0x26, 32}};
            instructions[".or-qword"] = {"byte, qword, qword", {0x26, 64}};
            instructions[".or-dqword"] = {"byte, dqword, dqword", {0x26, 128}};

            instructions[".xor-byte"] = {"byte, byte, byte", {0x25, 8}};
            instructions[".xor-word"] = {"byte, word, word", {0x25, 16}};
            instructions[".xor-dword"] = {"byte, dword, dword", {0x25, 32}};
            instructions[".xor-qword"] = {"byte, qword, qword", {0x25, 64}};
            instructions[".xor-dqword"] = {"byte, dqword, dqword", {0x25, 128}};

          // Data
          instructions[".db"] = {"byte", {}};
          instructions[".dw"] = {"word", {}};
          instructions[".dd"] = {"dword", {}};
          instructions[".dq"] = {"qword", {}};
          instructions[".dq128"] = {"dqword", {}};

          // Macros pour les registres (voir warch128_regs.hpp)
          // bmmx, wmmx, dmmx: 0-15
          for (int i = 0; i < 16; ++i) {
            macros["%bmm" + std::to_string(i)] = "byte("s + std::to_string(i) + ")";
            macros["%wmm" + std::to_string(i)] = "byte("s + std::to_string(i) + ")";
            macros["%dmm" + std::to_string(i)] = "byte("s + std::to_string(i) + ")";
          }
          // qmmx, dqmmx: 0-31
          for (int i = 0; i < 32; ++i) {
            macros["%qmm" + std::to_string(i)] = "byte("s + std::to_string(i) + ")";
            macros["%dqmm" + std::to_string(i)] = "byte("s + std::to_string(i) + ")";
          }
        }

      public:
        WylandAssembler() {

        }

        std::vector<uint8_t> compile_line(const std::string &line_raw, size_t line_number) {
          std::string line = trim(line_raw);
          if (line.empty() || line[0] == ';') return {};

          std::istringstream iss(line);
          std::string instr;
          iss >> instr;

          std::string args;
          std::getline(iss, args);
          args = trim(args);

          if (instr == ".array" || instr == ".data") {
            size_t bracket_start = line_raw.find('[');
            size_t bracket_end = line_raw.rfind(']');
            if (bracket_start == std::string::npos || bracket_end == std::string::npos || bracket_end <= bracket_start) {
              generate_error("Missing or misplaced brackets", line_raw, line_number, line_raw);
              return {};
            }
            std::string array_content = line_raw.substr(bracket_start, bracket_end - bracket_start + 1);
            return parse_array(trim(array_content), line_raw, line_number);
          } else if (instr == ".resbuntil") {
            size_t beg = line.find('(');
            size_t end = line.find(')', beg + 1);
            if (end == std::string::npos || beg == std::string::npos) {
              generate_error("excepted '(uint)' format.", line, line_number, line);
              return {};
            }

            std::string strval = line.substr(beg + 1, end - (beg + 1));
            try {
              unsigned long val = std::stoul(strval);
              std::vector<uint8_t> values{};
              if (val < current_address) return {};
              for (unsigned long i = current_address; i < val; i++) {
                values.push_back(0x00);
                current_address++;
              }

              return values;
            } catch (...) {
              generate_error("invalid format", line, line_number, line);
              return {};
            }
          } else if (instr == ".resb" || (line.starts_with(".resb") && !line.contains(':'))) {
            size_t beg = line.find('(');
            size_t end = line.find(')', beg + 1);
            if (end == std::string::npos || beg == std::string::npos) {
              generate_error("excepted '(uint)' format.", line, line_number, line);
              return {};
            }

            std::string strval = line.substr(beg + 1, end - (beg + 1));
            try {
              unsigned long val = std::stoul(strval);
              std::vector<uint8_t> values{};
              for (unsigned long i = 0; i < val; i++) {
                values.push_back(0x00);
                current_address++;
              }

              return values;
            } catch (...) {
              generate_error("invalid format", line, line_number, line);
              return {};
            }
          } else if (line.starts_with(".string")) {
            size_t beg = line.find('"');
            size_t end = line.find('"', beg + 1);
            if (beg == std::string::npos || end == std::string::npos) { generate_error("excepted double quotes", line, line_number, line); return{}; }
            std::string string = line.substr(beg + 1, end - (beg + 1));
            current_address += string.size();
            return wysm_binof(string);
          } else if (line.contains(':') && !line.starts_with(':')) {
            size_t end = line.find(':');
            if (end == std::string::npos) { generate_error("Excepted ':' token to end label's name", line, line_number, line); return{}; }
            std::string label_name = (line.substr(0, end));
            
            if (symbols.find(label_name) != symbols.end()) {
              generate_error("Redefinition of `" + label_name + "`", line, line_number, label_name);
              return {};
            } else {
              std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') << current_address << ": "
              "new symbol: `" << label_name << "`" << std::endl;
              symbols[label_name] = current_address;

              return {};
            }

            iss.str(line.substr(end + 1));
          } else if (symbols.find(instr) != symbols.end()) {
            current_address += 8;
            return wysm_binof(symbols[instr]);
          } else if (instructions.find(instr) == instructions.end()) {
            unresolved_references[instr].push_back(
              undefined_reference{
                .filepos = current_address,
                .line = line, 
                .line_count = line_number, 
                .refname = instr
              }
            );
            
            current_address += 8;
            return {wysm_binof<uint64_t>(0x00)};
          }
          
          std::vector<std::string> params;
          std::string token;
          std::stringstream ss(args);
          while (std::getline(ss, token, ',')) {
            params.push_back(trim(token));
          }
          
          const auto &[format, opcode] = instructions[instr];
          
          std::vector<std::string> expected;
          std::stringstream fmt(format);
          while (std::getline(fmt, token, ',')) {
            expected.push_back(trim(token));
          }
          
          if (params.size() != expected.size()) {
            generate_error("Wrong argument count excepted: " + std::to_string(expected.size()), line_raw, line_number, line);
            return {};
          }
          
          std::vector<uint8_t> result = opcode;
          current_address += result.size();
          for (size_t i = 0; i < params.size(); ++i) {
            const std::string &type = expected[i];
            const std::string &arg = params[i];
            
            size_t beg = arg.find('(');
            size_t end = arg.find(')');
            
            if (beg == std::string::npos || end == std::string::npos || beg >= end) {
              generate_error("Invalid argument format", line_raw, line_number, arg);
              return {};
            }
            
            std::string type_prefix = arg.substr(0, beg);
            std::string value_str = arg.substr(beg + 1, end - beg - 1);
            uint64_t value = 0;

            if (type_prefix != type) {
              generate_error("Argument type mismatch", line_raw, line_number, arg);
              return {};
            }
            
            if (value_str.starts_with("0x")) value = std::stoull(value_str, nullptr, 16); 
            else if (value_str.starts_with("0o")) value = std::stoull(value_str, nullptr, 8); 
            else if (is_numeric_string(value_str)) value = std::stoull(value_str, nullptr, 10);
            else {
              if (symbols.find(value_str) != symbols.end()) {
                value = symbols[value_str];
              }
              else {
                value = 0;
                unresolved_references[value_str].push_back(
                  undefined_reference {
                    .filepos = current_address,
                    .line = line, 
                    .line_count = line_number, 
                    .refname = value_str
                  }
                );
              }
            }
            
            if (type == "byte") {
              auto b = wysm_binof<uint8_t>(value);
              result.insert(result.end(), b.begin(), b.end());
              //if (!is_ref_def) unresolved_references[value_str].back().filepos += 1;
              current_address += 1;
            } else if (type == "word") {
              auto b = wysm_binof<uint16_t>(value);
              result.insert(result.end(), b.begin(), b.end());
              //if (!is_ref_def) unresolved_references[value_str].back().filepos += 2;
              current_address += 2;
            } else if (type == "dword") {
              auto b = wysm_binof<uint32_t>(value);
              result.insert(result.end(), b.begin(), b.end());
              //if (!is_ref_def) unresolved_references[value_str].back().filepos += 4;
              current_address += 4;
            } else if (type == "qword") {
              auto b = wysm_binof<uint64_t>(value);
              result.insert(result.end(), b.begin(), b.end());
              //if (!is_ref_def) unresolved_references[value_str].back().filepos += 8;
              current_address += 8;
            } else if (type == "dqword") {
              auto b = wysm_binof<__uint128_t>(value);
              result.insert(result.end(), b.begin(), b.end());
            } else {
              generate_error("Unsupported type", line_raw, line_number, type);
              return {};
            }
          }
          
          // current_address += result.size();

          return result;
        }

        std::string expand_macro(const std::string &line) {
          std::string expanded = line;
          bool changed = true;

          while (changed) {
            changed = false;
            for (const auto &[macro_name, macro_body] : macros) {
              size_t pos = expanded.find(macro_name);
              if (pos != std::string::npos) {
                expanded.replace(pos, macro_name.size(), macro_body);
                changed = true;
              }
            }
          }

          return expanded;
        }
        
        void resolve_reference(const std::string &refname, std::ofstream &stream) {
          if (symbols.find(refname) == symbols.end()) {
            auto &smth = unresolved_references[refname];

            for (const auto&idk:smth) {
              generate_error("Undefined reference to `" + idk.refname + "`", idk.line, idk.line_count, idk.refname);
            }

            return;
          }

          auto &smth = unresolved_references[refname];

          for (const auto&idk:smth) {
            std::cout << "resovled: " << refname << std::endl;
            stream.seekp(idk.filepos);
            auto bytes = wysm_binof(symbols[refname]);
            stream.write((char*)bytes.data(), bytes.size());
            stream.flush();
          }

          unresolved_references.erase(refname);
        }

        void compile_file(const std::string &filename, const std::string &outputfile,  const std::string &strarv, bool table = false, const std::string &arch = "") {
          Arch = arch;
          
          if (Arch == "x64") init_table_x64();
          else if (Arch == "x128") init_table_x128();
          // Else, dont init hahahhahahah
          std::ifstream in(filename);
          if (!in) {
            generate_error(("cannot open file " + filename), ("<argv>: " + strarv), 0, filename);
            return;
          }

          std::ofstream out(outputfile, std::ios::binary);
          std::string line;
          size_t line_num = 1;
          
          while (std::getline(in, line)) {
            line = (expand_macro(line));
            auto compiled = compile_line(line, line_num);
            out.write((char*)compiled.data(), compiled.size());
            line_num++;
            file_size += compiled.size();
          }

          std::cout << unresolved_references.size() << " symbols to resolve.." << std::endl;

          if (!unresolved_references.empty()) {
            std::vector<std::string> keys;
            for (const auto &[refname, _] : unresolved_references) {
              keys.push_back(refname);
            }
            for (const auto &refname : keys) {
              resolve_reference(refname, out);
            }
          }

          out.flush();
          out.seekp(current_address + 1);


          if (table) {
            char smth[] = "Symbol table:";
            out.write(smth, sizeof(smth));
            for (const auto&symbol:symbols) {
              auto addr = wysm_binof(symbol.second);
              out.write(symbol.first.c_str(), symbol.first.size());
              out.write(":", 1);
              out.write((char*)addr.data(), addr.size());
              out.write("\0", 1);
              file_size += symbol.first.size() + addr.size() + 2;
            }
          }

          if (file_size % 2 != 0) out.write("\0", 1);

          return;
        }

        size_t file_size = 0;
        static const constexpr long double version = 1.3;
      };

      void linify(const int argc, const char *const argv[], std::stringstream &ss) {
        for (int i = 0; i < argc; i++) {
          ss << argv[i] << " ";
        }
      }

      void linify(const std::vector<std::string> &vec, std::stringstream &ss) {
        for (const auto&arg:vec) ss << arg << " ";
      }

      std::string linify(const std::vector<std::string> &vec) {
        std::stringstream ss;
        for (const auto&arg:vec) ss << arg << " ";
        return ss.str();
      }

      std::string get_temp_file_name() {
        char temp_name[] = "tempfileXXXXXX";
      #ifdef _WIN32
        if (_mktemp_s(temp_name, sizeof(temp_name)) != 0) {
          throw std::runtime_error("Failed to create a temporary file name");
        }
      #else
        int fd = mkstemp(temp_name);
        if (fd == -1) {
          throw std::runtime_error("Failed to create a temporary file name");
        }
        close(fd);
      #endif
        return std::string(temp_name);
      }

      void copy_input_files(const std::string &to_file, const std::vector<std::string> &from_files, const std::string &linearg) {
        std::ofstream output(to_file);

        for (const auto &from_file:from_files) {
          std::ifstream file(from_file);
          if (!file) {
            generate_error("unable to open file " + from_file, linearg, 0, from_file);
            return;
          }

          std::string line;
          while (std::getline(file, line)) {
            output << line << std::endl;
          }

          file.close();
        }

        output.close();
      }

      void copy_output_files(const std::string &from_file, std::vector<std::string> &to_files, const std::string &linearg) {
        std::ifstream input(from_file, std::ios::binary);
        if (!input) {
          generate_error("unable to open file " + from_file, linearg, 0, from_file);
          return;
        }

        for (const auto &to_file : to_files) {
          input.clear();
          input.seekg(std::ios::beg);
          

          std::ofstream output(to_file, std::ios::binary);
          if (!output) {
            generate_error("unable to open file " + to_file, linearg, 0, to_file);
            return;
          }

          char buffer[2]{0};
          while (input.read(buffer, sizeof(buffer))) {
            output.write(buffer, sizeof(buffer));
          }

          output.close();
        }
        
        input.close();
      }

      void compile(const std::vector<std::string> &args) {
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        std::string Arch = "";

        bool verbose = false;
        bool include_table = false;

        for (size_t i = 0; i < args.size(); i++) {
          if (args[i] == "-v" || args[i] == "--verbose") verbose = true;
          else if (args[i] == "-table" ) include_table = true;
          else if (args[i] == "-o") {
            if (i + 1 >= args.size()) throw std::out_of_range("excepted argument after '-o'");
            auto out = args[++i];
            outputs.push_back(out);
          } else if (args[i] == "-arch") {
            if (i + 1 >= args.size()) throw std::out_of_range("excepted argument after '-arch'");
            Arch = args[++i];
          } else {
            inputs.push_back(args[i]);
          }
        }

        if (inputs.empty()) {
          generate_error("excepted input file", linify(args), 0, "");
          return;
        }

        std::string input = get_temp_file_name();
        std::string output = get_temp_file_name();

        copy_input_files(input, inputs, linify(args));

        WylandAssembler wysm;
        if (!verbose) std::cout.setstate(std::ios::failbit);
        std::cout << "starting compilation" << std::endl;

        wysm.compile_file(input, output, linify(args), include_table, Arch);
        std::cout << "output size: ";

        if (wysm.file_size < 1_KB) {
          std::cout << std::dec << wysm.file_size << "b";
        } else if (wysm.file_size > 1_KB) {
          std::cout << std::dec << wysm.file_size / 1_KB << "Kb";
        } else if (wysm.file_size > 1_MB) {
          std::cout << std::dec << wysm.file_size / 1_MB << "Mb";
        } else if (wysm.file_size > 1_GB) {
          std::cout << std::dec << wysm.file_size / 1_GB << "Gb";
        }

        std::cout << std::endl;
        copy_output_files(output, outputs, linify(args));

        std::cout.clear();

        std::filesystem::remove(input);
        std::filesystem::remove(output);
      }
    }
  }
}