cmake_minimum_required(VERSION 3.20)
project(WylandProject C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)

# === STRUCTURE ===
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_ROOT}")
include_directories(${INCLUDE_DIR})

# === OPTIONS ===
option(USE_STACKTRACE "Enable stacktrace" OFF)
option(USE_SWITCH "Enable switch instructions" OFF)
option(FLOAT128 "Use float128 with GCC 14+" OFF)
set(BUILD_LINK_MODE "HYBRID" CACHE STRING "Link mode: STATIC, DYNAMIC, HYBRID")

# === FLAGS ===
set(COMMON_FLAGS -O3 -Wall -Wextra -funroll-loops -flto=auto -ferror-limit=no)
set(DEBUG_FLAGS -g -D__WYLAND_DEBUG__ -D_GNU_SOURCE -DWYLAND_STACKTRACE)
set(STACKTRACE_FLAGS -D_GNU_SOURCE -DWYLAND_STACKTRACE)
set(SHARED_OBJECT_FLAGS -shared -fPIC)
set(SWITCH_FLAGS -D___WYLAND_SWITCH_INSTRUCTIONS___)

# === COMPILER ===
if (FLOAT128)
    set(CMAKE_C_COMPILER "gcc-14")
    set(CMAKE_CXX_COMPILER "g++-14")
    add_compile_options(-D_Alignof=alignof -D___WYLAND_GNU_USE_FLOAT128___)
    link_libraries(quadmath)
endif()

# === BUILD FLAGS ===
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${DEBUG_FLAGS})
elseif (USE_STACKTRACE)
    add_compile_options(${STACKTRACE_FLAGS})
endif()

if (USE_SWITCH)
    add_compile_options(${SWITCH_FLAGS})
endif()

# === DIRECTORIES ===
set(BIN_DIR ${PROJECT_ROOT}/bin)
set(RELEASE_DIR ${BIN_DIR}/release)
set(DEBUG_DIR ${BIN_DIR}/debug)
file(MAKE_DIRECTORY ${RELEASE_DIR})
file(MAKE_DIRECTORY ${DEBUG_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEBUG_DIR})
    set(OBJ_OUTPUT_DIR ${DEBUG_DIR})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RELEASE_DIR})
    set(OBJ_OUTPUT_DIR ${RELEASE_DIR})
endif()

# === SOURCES ===
set(CCACHE_SRC     ${PROJECT_ROOT}/ccache.cpp)
set(CACHE_SRC      ${PROJECT_ROOT}/cache.cpp)
set(EXTERN_DEF_SRC ${PROJECT_ROOT}/externdef.cpp)
set(BIOS_SRC       ${PROJECT_ROOT}/bios/bios_backend.cpp)
set(BINDINGS_SRC   ${PROJECT_ROOT}/wyland-runtime/bindings.cpp)
set(RUNTIME_SRC    ${PROJECT_ROOT}/wyland-runtime/wylrt.c)
set(WYLAND_SRC     ${PROJECT_ROOT}/wyland.cpp)

# === OBJECT LIBS ===
function(add_shared_object name srcs)
    add_library(${name} OBJECT ${srcs})
    target_compile_options(${name} PRIVATE ${SHARED_OBJECT_FLAGS})
    target_compile_definitions(${name} PRIVATE ___WYLAND_NOT_MAIN_BUILD___)
    set_target_properties(${name} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            OUTPUT_NAME ${name}
            ARCHIVE_OUTPUT_DIRECTORY ${OBJ_OUTPUT_DIR}
    )
endfunction()

add_shared_object(ccache_obj     ${CCACHE_SRC})
add_shared_object(cache_obj      ${CACHE_SRC})
add_shared_object(extern_def_obj ${EXTERN_DEF_SRC})
add_shared_object(bios_obj       ${BIOS_SRC})
add_shared_object(bindings_obj   ${BINDINGS_SRC})
add_shared_object(runtime_obj    ${RUNTIME_SRC})

# === OBJECT TARGET LIST ===
set(WYLAND_OBJS
        $<TARGET_OBJECTS:ccache_obj>
        $<TARGET_OBJECTS:cache_obj>
        $<TARGET_OBJECTS:extern_def_obj>
        $<TARGET_OBJECTS:bios_obj>
        $<TARGET_OBJECTS:bindings_obj>
        $<TARGET_OBJECTS:runtime_obj>
)

# === EXECUTABLE ===
if (BUILD_LINK_MODE STREQUAL "STATIC")
    message(STATUS "Building wyland statically")
    add_executable(wyland ${WYLAND_SRC} ${WYLAND_OBJS})

elseif (BUILD_LINK_MODE STREQUAL "DYNAMIC")
    message(STATUS "Building wyland dynamically (object files will be external)")
    add_executable(wyland ${WYLAND_SRC}) # No object linkage
    # Copy .o files to bin dir
    add_custom_command(TARGET wyland POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:ccache_obj>     ${OBJ_OUTPUT_DIR}/ccache.o
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:cache_obj>      ${OBJ_OUTPUT_DIR}/cache.o
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:extern_def_obj> ${OBJ_OUTPUT_DIR}/externdef.o
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:bios_obj>       ${OBJ_OUTPUT_DIR}/bios_backend.o
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:bindings_obj>   ${OBJ_OUTPUT_DIR}/bindings.o
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:runtime_obj>    ${OBJ_OUTPUT_DIR}/wylrt.o
    )
else()
    message(STATUS "Building wyland in hybrid mode")
    add_executable(wyland ${WYLAND_SRC} ${WYLAND_OBJS})
endif()

add_compile_definitions(__WYLAND_BUILD_MODE="${BUILD_LINK_MODE}")
