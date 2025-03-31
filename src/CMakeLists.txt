cmake_minimum_required(VERSION 3.10)

# Set policy CMP0167 to NEW to handle the removal of FindBoost module
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Use BoostConfig.cmake instead of FindBoost
set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.87.0_1)
set(BOOST_INCLUDEDIR /opt/homebrew/Cellar/boost/1.87.0_1/include)
set(BOOST_LIBRARYDIR /opt/homebrew/Cellar/boost/1.87.0_1/lib)

find_package(Boost REQUIRED CONFIG)

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
add_compile_definitions(_GNU_SOURCE)

# Find and include Boost
set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.87.0_1)
set(BOOST_INCLUDEDIR /opt/homebrew/Cellar/boost/1.87.0_1/include)
set(BOOST_LIBRARYDIR /opt/homebrew/Cellar/boost/1.87.0_1/lib)

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(WylandExecutable ${Boost_LIBRARIES})

# Specify the platform-specific output directory for object files
set_target_properties(WylandExecutable PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME}
)