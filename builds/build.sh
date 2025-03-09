#!/bin/bash

# Detect compiler (g++-14 or clang++)
CXX=""
if command -v g++-14 &>/dev/null; then
    CXX="g++-14"
elif command -v clang++ &>/dev/null; then
    CXX="clang++"
else
    echo "No compiler found! Please install g++-14 or clang++."
    exit 1
fi

# Create build directory
mkdir -p build

echo "Compiling with $CXX"

# Compile for macOS (Universal binary: x86_64 + ARM64)
echo "Compiling for macOS (x86_64 + ARM64)..."
$CXX -arch x86_64 -arch arm64 -o build/wylandMacOS "../wyland.cpp" -std=c++23

# Compile for Linux 64-bit
echo "Compiling for Linux 64-bit..."
x86_64-elf-gcc -o build/WylandLinux64 "../wyland.cpp" -std=c++23

# Compile for Linux 32-bit
echo "Compiling for Linux 32-bit..."
i686-elf-gcc -m32 -o build/WylandLinux32 "../wyland.cpp" -std=c++23

# Compile for Windows 64-bit
echo "Compiling for Windows 64-bit..."
x86_64-w64-mingw32-g++ -o build/wylandWin64.exe "../wyland.cpp" -std=c++23

# Compile for Windows 32-bit
echo "Compiling for Windows 32-bit..."
i686-w64-mingw32-g++ -o build/wylandWin32.exe "../wyland.cpp" -std=c++23

echo "Compilation completed! Files are in the 'build/' directory." -std=c++23
ls -lh build/
