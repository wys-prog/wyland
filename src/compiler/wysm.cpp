#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <iomanip>

#include "iwysm.hpp"

using namespace wylma::wyland::wysm;

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <input.asm> -o <output.bin>\n";
		return 1;
	}

	bool include_symbol_table = false;
	bool verbose = false;
	std::string input, output = "out.bin";
	std::stringstream ss;
	linify(argc, argv, ss);

	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "-o") {
			if (i + 1 >= argc) {
				std::cerr << "[e]: excepted file after -o" << std::endl;
				return -1;
			} else {
				output = argv[++i];
			}
		} else if (std::string(argv[i]) == "-table") include_symbol_table = true;
		else if (std::string(argv[i]) == "-v") verbose = true;
		else input = argv[i];
	}

	if (!verbose) {
		std::cout.setstate(std::ios_base::failbit);
	}

	WylandAssembler compiler;
	compiler.compile_file(input, output, ss.str(), include_symbol_table);
	
	if (errors > 0) {
		std::cerr << "Aborted due to errors." << std::endl;
		return 1;
	}

	std::cout << "Compiled, output size: " << std::dec << compiler.file_size << " bytes" << std::endl;

	return 0;
}
