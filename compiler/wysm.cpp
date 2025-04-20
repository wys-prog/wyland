#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <cctype>
#include <algorithm>

// ==== UTILS ====
template <typename T>
std::vector<uint8_t> binof(T value) {
	std::vector<uint8_t> result(sizeof(T));
	for (size_t i = 0; i < sizeof(T); ++i) {
		result[sizeof(T) - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
	}
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
	std::cerr << "error: " << what << "\n\t| " << line_count << ":" << line << "\n\t|  ";
	for (size_t i = 0; i < std::to_string(line_count).size(); i++) std::cout << ' ';
	size_t beg = line.find(word);
	if (beg == std::string::npos) beg = 0;
	for (size_t i = 0; i < beg; i++) std::cout << ' ';
	for (size_t i = 0; i < word.size(); i++) std::cout << '~';
	std::cout << std::endl;
	errors++;
}

// ==== COMPILER ====
class AutoAssembler {
	uint64_t current_address = 0;
	std::unordered_map<std::string, std::pair<std::string, std::vector<uint8_t>>> instructions;
	std::unordered_map<std::string, uint8_t> registers;
	std::unordered_map<std::string, uint64_t> symbols;

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
				auto b = binof(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "word") {
				uint16_t value = static_cast<uint16_t>(std::stoul(val, nullptr, 16));
				auto b = binof(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "dword") {
				uint32_t value = static_cast<uint32_t>(std::stoul(val, nullptr, 16));
				auto b = binof(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "qword") {
				uint64_t value = static_cast<uint64_t>(std::stoull(val, nullptr, 16));
				auto b = binof(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "char") {
				if (val.size() == 3 && val.front() == '\'' && val.back() == '\'') {
					char c = val[1];
					result.push_back(static_cast<uint8_t>(c));
				} else {
					generate_error("Invalid char format. Expected 'X'", line, line_num, val);
					return {};
				}
			} else if (type == "string") {
				if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
					for (size_t i = 1; i < val.size() - 1; ++i) {
						result.push_back(static_cast<uint8_t>(val[i]));
					}
				} else {
					generate_error("Invalid string format. Expected \"...\"", line, line_num, val);
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
	

public:
	AutoAssembler() {
		// Define instruction format and opcode (1 byte)
		instructions[".nop"] = {"", {0x00}};
		instructions[".lea"] = {"qword", {0x01}};
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
		instructions[".int"] = {"", {18}};
		instructions[".loadat"] = {"byte, qword", {19}};
		instructions[".ret"] = {"", {20}};
		instructions[".movad"] = {"byte, byte", {21}};
		instructions[".sal"] = {"byte, qword", {22}};
		instructions[".sar"] = {"byte, qword", {23}};
		instructions[".throw"] = {"", {24}};
		instructions[".clfn"] = {"dword", {25}};
		instructions[".call-c"] = {"dword", {25}};
		instructions[".emplace"] = {"qword, byte", {26}};
		instructions[".pushmmio"] = {"byte, qword", {27}};
		instructions[".popmmio"] = {"byte", {28}};
		// Define registers
		for (int i = 0; i <= 15; ++i) {
			registers["%bmm" + std::to_string(i)] = (i);
			registers["%wmm" + std::to_string(i)] = (16 + i);
			registers["%dmm" + std::to_string(i)] = (32 + i);
		}

		for (int i = 0; i <= 31; ++i) {
			registers["%qmm" + std::to_string(i)] = (48 + i);
		}
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

		if (instr == ".array") {
			size_t bracket_start = line_raw.find('[');
			size_t bracket_end = line_raw.rfind(']');
			if (bracket_start == std::string::npos || bracket_end == std::string::npos || bracket_end <= bracket_start) {
				generate_error("Missing or misplaced brackets in .array", line_raw, line_number, line_raw);
				return {};
			}
			std::string array_content = line_raw.substr(bracket_start, bracket_end - bracket_start + 1);
			return parse_array(trim(array_content), line_raw, line_number);
		}

		if (instr == ".l") {
			std::string label_name;
			iss >> label_name;
			if (label_name.ends_with(':')) label_name.pop_back();
			if (symbols.find(label_name) != symbols.end()) {
				generate_error("Redefinition of `" + label_name + "`", line, line_number, label_name);
				return {};
			} else {
				symbols[label_name] = current_address;
				return {};
			}
		} else if (symbols.find(instr) != symbols.end()) {
			current_address += 8;
			return binof(symbols[instr]);
		} else if (registers.find(instr) != registers.end()) {
			current_address += 1;
			return binof(registers[instr]);
		} else if (instructions.find(instr) == instructions.end()) {
			generate_error("Unknown instruction", line_raw, line_number, instr);
			return {};
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
			generate_error("Wrong argument count", line_raw, line_number, line);
			return {};
		}
		
		std::vector<uint8_t> result = opcode;
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
			uint64_t value = std::stoull(value_str, nullptr, 16);
			
			if (type_prefix != type) {
				generate_error("Argument type mismatch", line_raw, line_number, arg);
				return {};
			}
			
			if (type == "byte") {
				auto b = binof<uint8_t>(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "word") {
				auto b = binof<uint16_t>(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "dword") {
				auto b = binof<uint32_t>(value);
				result.insert(result.end(), b.begin(), b.end());
			} else if (type == "qword") {
				auto b = binof<uint64_t>(value);
				result.insert(result.end(), b.begin(), b.end());
			} else {
				generate_error("Unsupported type", line_raw, line_number, type);
				return {};
			}
		}

		current_address += result.size();
		
		return result;
	}
	
	void compile_file(const std::string &filename, const std::string &outputfile) {
		std::ifstream in(filename);
		std::ofstream out(outputfile);
		std::string line;
		size_t line_num = 1;
		
		while (std::getline(in, line)) {
			auto compiled = compile_line(line, line_num);
			out.write((char*)compiled.data(), compiled.size());
			line_num++;
			file_size += compiled.size();
		}
		
		return;
	}

	size_t file_size = 0;
};

// ==== MAIN ====
int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./assembler <input.asm> <output.bin>\n";
		return 1;
	}

	AutoAssembler compiler;
	compiler.compile_file(argv[1], argv[2]);

	if (errors > 0) {
		std::cerr << "Aborted due to errors." << std::endl;
		return 1;
	}

	std::cout << "Compiled, output size: " << compiler.file_size << " bytes" << std::endl;

	return 0;
}
