cmake_minimum_required(VERSION 3.20)
project(WylandProject C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_ROOT}")
include_directories(${INCLUDE_DIR})

set(COMMON_FLAGS -O3 -Wall -Wextra -funroll-loops -flto=auto -ferror-limit=no)
set(DEBUG_FLAGS -g -D__WYLAND_DEBUG__ -D_GNU_SOURCE -DWYLAND_STACKTRACE)
set(STACKTRACE_FLAGS -D_GNU_SOURCE -DWYLAND_STACKTRACE)
set(SHARED_FLAGS -shared -fPIC)
set(SWITCH_FLAGS -D___WYLAND_SWITCH_INSTRUCTIONS___)

option(USE_STACKTRACE "Enable stacktrace" OFF)
option(USE_SWITCH "Enable switch instructions" OFF)
option(FLOAT128 "Use float128 with GCC 14+" OFF)

if (FLOAT128)
    set(CMAKE_C_COMPILER "gcc-14")
    set(CMAKE_CXX_COMPILER "g++-14")
    add_compile_options(-D_Alignof=alignof -D___WYLAND_GNU_USE_FLOAT128___)
    link_libraries(quadmath)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${DEBUG_FLAGS})
elseif(USE_STACKTRACE)
    add_compile_options(${STACKTRACE_FLAGS})
endif()

if (USE_SWITCH)
    add_compile_options(${SWITCH_FLAGS})
endif()

set(BIN_DIR ${PROJECT_ROOT}/bin)
set(RELEASE_DIR ${BIN_DIR}/release)
set(DEBUG_DIR ${BIN_DIR}/debug)

file(MAKE_DIRECTORY ${RELEASE_DIR})
file(MAKE_DIRECTORY ${DEBUG_DIR})

set(EXTERN_DEF_SRC  ${PROJECT_ROOT}/externdef.cpp)
set(BIOS_SRC ${PROJECT_ROOT}/bios/bios_backend.cpp)
set(BINDINGS_SRC ${PROJECT_ROOT}/wyland-runtime/bindings.cpp
        src/def/usbEXdef.hpp)
set(RUNTIME_SRC ${PROJECT_ROOT}/wyland-runtime/wylrt.c)
set(WYLAND_SRC ${PROJECT_ROOT}/wyland.cpp)
set(UPDATER_SRC ${PROJECT_ROOT}/updates/update.cpp)
set(CACHE_SRC ${PROJECT_ROOT}/cache.cpp)

set(SHARED_OBJ_TARGETS extern_def_obj bios_obj bindings_obj runtime_obj updater_obj)

add_library(extern_def_obj OBJECT ${EXTERN_DEF_SRC})
target_compile_options(extern_def_obj PRIVATE ${SHARED_FLAGS})
target_compile_definitions(extern_def_obj PRIVATE ___WYLAND_NOT_MAIN_BUILD___)

add_library(bios_obj OBJECT ${BIOS_SRC})
target_compile_options(bios_obj PRIVATE ${SHARED_FLAGS})
target_compile_definitions(bios_obj PRIVATE ___WYLAND_NOT_MAIN_BUILD___)

add_library(bindings_obj OBJECT ${BINDINGS_SRC})
target_compile_options(bindings_obj PRIVATE ${SHARED_FLAGS})
target_compile_definitions(bindings_obj PRIVATE ___WYLAND_NOT_MAIN_BUILD___)

add_library(runtime_obj OBJECT ${RUNTIME_SRC})
target_compile_options(runtime_obj PRIVATE ${SHARED_FLAGS})
target_compile_definitions(runtime_obj PRIVATE ___WYLAND_NOT_MAIN_BUILD___)

add_library(updater_obj OBJECT ${UPDATER_SRC})
target_compile_options(updater_obj PRIVATE ${SHARED_FLAGS})
target_compile_definitions(updater_obj PRIVATE ___WYLAND_NOT_MAIN_BUILD___)

add_library(cache_obj OBJECT ${CACHE_SRC})
target_compile_options(cache_obj PRIVATE ${SHARED_FLAGS})
target_compile_definitions(cache_obj PRIVATE ___WYLAND_NOT_MAIN_BUILD___)

add_executable(wyland ${WYLAND_SRC} $<TARGET_OBJECTS:extern_def_obj> $<TARGET_OBJECTS:bios_obj> $<TARGET_OBJECTS:bindings_obj> $<TARGET_OBJECTS:runtime_obj>
        src/def/usbEXdef.hpp)
add_executable(updater $<TARGET_OBJECTS:updater_obj> $<TARGET_OBJECTS:bindings_obj> $<TARGET_OBJECTS:runtime_obj>)
add_executable(cache $<TARGET_OBJECTS:cache_obj>)

target_link_libraries(updater PRIVATE curl)

set(VERSION_HEADER "${PROJECT_ROOT}/data/wyland_version.h")
set(VERSION_FILE "${PROJECT_ROOT}/data/build_id.txt")
