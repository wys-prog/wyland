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


%macro nop 0
    db 0
%endmacro

%macro lea 2
    db 1
    db %1
    dq %2
%endmacro

%macro load 3
    db 2
    db %1
    db %2
    dq %3
%endmacro

%macro store 3
    db 3
    db %1
    db %2
    dq %3
%endmacro

%macro mov 2
    db 4
    db %1
    db %2
%endmacro

%macro add 2
    db 5
    db %1
    db %2
%endmacro

%macro sub 2
    db 6
    db %1
    db %2
%endmacro

%macro mul 2
    db 7
    db %1
    db %2
%endmacro

%macro odiv 2
    db 8
    db %1
    db %2
%endmacro

%macro mod 2
    db 9
    db %1
    db %2
%endmacro

%macro jmp 1
    db 10
    dq %1
%endmacro

%macro je 1
    db 11
    dq %1
%endmacro

%macro jne 1
    db 12
    dq %1
%endmacro

%macro jl 1
    db 13
    dq %1
%endmacro

%macro jg 1
    db 14
    dq %1
%endmacro

%macro jle 1
    db 15
    dq %1
%endmacro

%macro jge 1
    db 16
    dq %1
%endmacro

%macro cmp 2
    db 17
    db %1
    db %2
%endmacro

%macro xint 1
    db 18
    db %1
%endmacro

%macro loadat 2
    db 19
    db %1
    dq %2
%endmacro

%macro ret 0
    db 20
%endmacro

%macro movad 2
    db 21
    db %1
    db %2
%endmacro

%macro writec 0
    db 0
%endmacro

%macro writerc 0
    db 1
%endmacro

%macro readc 0
    db 2
%endmacro

%macro csystem 0
    db 3
%endmacro

%macro sldlib 0
    db 4
%endmacro

%macro startt 0
    db 5
%endmacro

%macro pseg 0
    db 6
%endmacro

%macro reads 0
    db 7
%endmacro

%macro sldlcfun 0
    db 8
%endmacro

%macro suldlib 0
    db 9
%endmacro

%macro scfun 0
    db 10
%endmacro
