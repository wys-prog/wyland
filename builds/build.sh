#!/bin/bash

# Create build directory
mkdir -p build

echo "Compiling with $CXX"

# Compile for macOS (Universal binary: x86_64 + ARM64)
echo "Compiling for macOS (x86_64 + ARM64)..."
clang++ -arch x86_64 -arch arm64 -o build/wylandMacOS "../wyland.cpp" -std=c++23  -Wall -Wextra -O3 #-funroll-loops

# Compile for Linux 64-bit
echo "Compiling for Linux 64-bit..."
x86_64-elf-gcc -o build/WylandLinux64 "../wyland.cpp" -std=c++23  -Wall -Wextra -funroll-loops

# Compile for Linux 32-bit
echo "Compiling for Linux 32-bit..."
i686-elf-gcc -m32 -o build/WylandLinux32 "../wyland.cpp" -std=c++23  -Wall -Wextra -funroll-loops

# Compile for Windows 64-bit
echo "Compiling for Windows 64-bit..."
x86_64-w64-mingw32-g++ -o build/wylandWin64.exe "../wyland.cpp" -std=c++23 -static -O3 -Wall -Wextra -funroll-loops

# Compile for Windows 32-bit
echo "Compiling for Windows 32-bit..."
i686-w64-mingw32-g++ -o build/wylandWin32.exe "../wyland.cpp" -std=c++23 -static -O3  -Wall -Wextra -funroll-loops

echo "Compilation completed! Files are in the 'build/' directory."
ls -lh build/
