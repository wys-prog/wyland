# this script is built by an IA because I don't really know exactly how Make and CMake work, so for 
# the moment I use them.
# I promise I'll try to do it by myself as soon as possible ! 
# If you have some suggestions or other (idk), you can tell me !
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

set(WYLAND
    main.cpp
)

# Add executables
add_executable(WylandExecutable ${SOURCE_FILES})
add_executable(wyland ${WYLAND})
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
    target_link_libraries(wyland ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost not found! Install Boost or set BOOST_ROOT manually.")
endif()

# Set platform-specific output directories for both executables
set_target_properties(WylandExecutable PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME}
)

set_target_properties(wyland PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/obj/${CMAKE_SYSTEM_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME}
)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")
file(MAKE_DIRECTORY ${DATA_DIR})

set(BUILD_ID_FILE "${DATA_DIR}/build_id.txt")
set(VERSION_HEADER "${DATA_DIR}/wyland_version.h")
set(SCRIPT "${CMAKE_SOURCE_DIR}/scripts/increment_build.py")

add_custom_command(
    OUTPUT ${VERSION_HEADER}
    #COMMAND ${CMAKE_COMMAND} -E echo ">> Creating something idk..."
    COMMAND python3 ${SCRIPT} ${BUILD_ID_FILE} ${VERSION_HEADER}
    DEPENDS ${SCRIPT}
    COMMENT "Incrementing build version"
)

add_custom_target(run_version_script ALL
    COMMAND python3 ${SCRIPT} ${BUILD_ID_FILE} ${VERSION_HEADER}
    COMMENT "Always running version increment script"
    BYPRODUCTS ${VERSION_HEADER}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(generate_version_header DEPENDS ${VERSION_HEADER})
add_dependencies(WylandExecutable generate_version_header)
target_include_directories(WylandExecutable PRIVATE ${DATA_DIR})
