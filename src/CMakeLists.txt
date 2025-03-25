cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(WylandProject VERSION 1.0)

# Specify the C and C++ standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

# Set the output directories for object files and binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Specify the source files
set(SOURCE_FILES
  wyland-runtime/wylrt.c
  wyland-runtime/bindings.cpp
  wyland.cpp
)

# Add the executable
add_executable(WylandExecutable ${SOURCE_FILES})

# Specify the platform-specific output directory for object files
set_target_properties(WylandExecutable PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME}
)