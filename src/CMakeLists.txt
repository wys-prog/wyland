cmake_minimum_required(VERSION 3.10)

# Set policy CMP0167 to NEW to handle the removal of FindBoost module
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Set the project name and version
project(WylandProject VERSION 1.0)

# Specify the C and C++ standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

# Set output directories for object files and binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(SOURCE_FILES
    wyland-runtime/wylrt.c
    wyland-runtime/bindings.cpp
    wyland.cpp
)

# Add executable
add_executable(WylandExecutable ${SOURCE_FILES})
add_compile_definitions(_GNU_SOURCE)

# Detect platform
if(APPLE)
    message(STATUS "Building for macOS")
    set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.87.0_1)
    set(BOOST_INCLUDEDIR /opt/homebrew/Cellar/boost/1.87.0_1/include)
    set(BOOST_LIBRARYDIR /opt/homebrew/Cellar/boost/1.87.0_1/lib)

elseif(UNIX)
    message(STATUS "Building for Linux")
    set(BOOST_ROOT /usr/local)
    set(BOOST_INCLUDEDIR /usr/include)
    set(BOOST_LIBRARYDIR /usr/lib)

elseif(WIN32)
    message(STATUS "Building for Windows")
    set(BOOST_ROOT "C:/Boost/boost_1_87_0")
    set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
endif()

# Find and link Boost
find_package(Boost REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(WylandExecutable ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost not found! Install Boost or set BOOST_ROOT manually.")
endif()

# Set platform-specific output directories
set_target_properties(WylandExecutable PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME}
)
