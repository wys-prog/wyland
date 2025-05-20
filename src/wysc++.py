#!/usr/bin/env python3

import os
import sys
import subprocess
from pathlib import Path

platform = None

args = sys.argv[1:]
force_os = None

# Check if --os argument is specified
if '--os' in args:
    os_index = args.index('--os') + 1
    if os_index < len(args):
        force_os = args[os_index].lower()
        args.remove('--os')
        args.remove(args[os_index - 1])  # remove --os argument

if force_os:
    platform = force_os
elif sys.platform == "darwin":
    platform = "mac"
elif sys.platform == "linux":
    platform = "linux"
else:
    platform = "windows"

# === Windows (MinGW) ===
mingw32 = "i686-w64-mingw32-gcc-14.2.0"
mingw64 = "x86_64-w64-mingw32-gcc-14.2.0"

# === Docker for Linux ===
docker_image = "debian:bookworm"
container_name = ''

infile = Path('./data/build_id.txt')
outfile = Path('./data/wyland_version.h')

major = 1

if infile.exists():
    try:
        build = int(infile.read_text().strip())
    except ValueError:
        build = 0
else:
    build = 0

build += 1
old_build = build
minor = int(build / 1000)
build = int(old_build)
infile.write_text(str(old_build))

version_str = f"{major}.{minor:01}.{build:04}"
uversion_str = f'{major}{minor}'

with open(outfile, 'w') as f:
    f.write(f'#ifndef ___WYLAND_BUILD_SERVICE___\n')
    f.write(f'#define ___WYLAND_BUILD_SERVICE___\n')
    f.write(f'#define WYLAND_VERSION "{version_str}"\n')
    f.write(f'#define WYLAND_VERSION_UINT32 {uversion_str}\n')
    f.write(f'#define WYLAND_VERSION_MAJOR_STR "{major}"\n')
    f.write(f'#define WYLAND_VERSION_MINOR_STR "{minor}"\n')
    f.write(f'#define WYLAND_VERSION_MAJOR_INT {major}\n')
    f.write(f'#define WYLAND_VERSION_MINOR_INT {minor}\n')
    f.write(f'#endif\n')

# === CONFIGURATION ===
project_root = './'
out_base = f'{project_root}/bin'

release_dir = f'{out_base}/release'
debug_dir = f'{out_base}/debug'

os.makedirs(release_dir, exist_ok=True)
os.makedirs(debug_dir, exist_ok=True)

# Compilers
cxxclang = 'clang++'
ccclang = 'clang'
cxxgnu = 'g++-14 -D_Alignof=alignof -D___WYLAND_GNU_USE_FLOAT128___ -lquadmath'
ccgnu = 'gcc-14 -D_Alignof=alignof -D___WYLAND_GNU_USE_FLOAT128___ -lquadmath'
cxx = ''
cc = ''

if platform == "windows":
    cxx = mingw64  
    cc = mingw32

# Flags
common_flags = "-O3 -Wall -Wextra -flto=auto -funroll-loops"
shared_flags = "-shared -fPIC"
cpp_std = "-std=c++23"
c_std = "-std=c17"
flags_stacktrace = "-D_GNU_SOURCE -DWYLAND_STACKTRACE"
debug_flag = f'-g -D__WYLAND_DEBUG__ {flags_stacktrace} -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer'
includes = f"-I{project_root}"
flags_switch = '-D___WYLAND_SWITCH_INSTRUCTIONS___'
use_switch = False

# Source files
bios_src = f"{project_root}/bios/bios_backend.cpp -D___WYLAND_NOT_MAIN_BUILD___"
bindings_src = f"{project_root}/wyland-runtime/bindings.cpp -D___WYLAND_NOT_MAIN_BUILD___"
runtime_src = f"{project_root}/wyland-runtime/wylrt.c -D___WYLAND_NOT_MAIN_BUILD___"
wyland_src = f"{project_root}/wyland.cpp"
updater_src = f"{project_root}/updates/update.cpp -D___WYLAND_NOT_MAIN_BUILD___"

# === Configuration ===

enable_stacktrace = False

# === Build functions ===
def build_bindings(build_type):
    out_dir = release_dir if build_type == "release" else debug_dir
    output = f"{out_dir}/bindings.o"
    flags = f"{cxx} {bindings_src} {runtime_src} {common_flags} {cpp_std} {includes} {shared_flags}"
    if build_type == "debug":
        flags += f" {debug_flag}"
    return f"{flags} -o {output}"

def build_runtime(build_type):
    out_dir = release_dir if build_type == "release" else debug_dir
    output = f"{out_dir}/runtime.o"
    bindings = f"{out_dir}/bindings.o"
    flags = f"{cc} {runtime_src} {bindings} {common_flags} {c_std} {includes} {shared_flags}"
    if build_type == "debug":
        flags += f" {debug_flag}"
    return f"{flags} -o {output}"

def build_bios(build_type):
    out_dir = release_dir if build_type == "release" else debug_dir
    output = f"{out_dir}/bios.o"
    flags = f"{cxx} {bios_src} {common_flags} {cpp_std} {includes} {shared_flags}"
    if build_type == "debug":
        flags += f" {debug_flag}"
    return f"{flags} -o {output}"

def build_wyland(build_type):
    out_dir = release_dir if build_type == "release" else debug_dir
    output = f"{out_dir}/wyland"
    bios = f"{out_dir}/bios.o"
    bindings = f"{out_dir}/bindings.o"
    runtime = f"{out_dir}/runtime.o"
    
    flags = f"{cxx} {wyland_src} {bios} {bindings} {runtime} {common_flags} {cpp_std} {includes} {flags_switch if use_switch else ''} "
    if build_type == "debug":
        flags += f" {debug_flag}"
    elif enable_stacktrace: flags += f" {flags_stacktrace}"
    return f"{flags} -o {output}"

def build_updater(build_type):
    out_dir = release_dir if build_type == "release" else debug_dir
    output = f"{out_dir}/updater"
    
    flags = f"{cxx} {updater_src} {runtime_src} {bindings_src} {common_flags} {cpp_std} {includes} -lcurl"
    if build_type == "debug":
        flags += f" {debug_flag}"
    elif enable_stacktrace: flags += f" {flags_stacktrace}"
    return f"{flags} -o {output}"

# === Command execution ===
build_map = {
    "bios": build_bios,
    "bindings": build_bindings,
    "runtime": build_runtime,
    "wyland": build_wyland,
    "updater": build_updater,
}

# === Argument parsing ===
build_types = ["release", "debug"]
selected_types = set(build_types)
default_order = ["bios", "bindings", "runtime", "wyland", "updater"]
selected_targets = default_order.copy()
cc = ccclang
cxx = cxxclang

for arg in args:
    if arg in build_types:
        selected_types = {arg}
    elif arg in build_map:
        selected_targets = [arg]
    elif arg == "clean":
        print("Cleaning output directories...")
        os.system(f"rm -rf {release_dir}/* {debug_dir}/*")
        sys.exit(0)
    elif arg.lower() == 'switch':
        use_switch = True
    elif arg.lower() == 'cc=gcc':
        cc = ccgnu
    elif arg.lower() == 'cxx=g++':
        cxx = cxxgnu
    elif arg.lower() == 'cxx=win32':
        cc = mingw32
    elif arg.lower() == 'cxx=win64':
        cxx = mingw64
    elif arg.lower() == 'stack':
        enable_stacktrace = True
    else:
        print(f"Unknown argument: {arg}")
        sys.exit(1)

# === Check if Docker container exists ===
def check_and_create_docker_container():
    container_name = "wyland-dev-container"
    
    result = subprocess.run(["docker", "ps", "-a", "--filter", f"name={container_name}"], capture_output=True, text=True)
    if container_name not in result.stdout:
        print("Creating a new Docker container with necessary tools...")
        
        subprocess.run(f"docker run -d --name {container_name} {docker_image} bash -c "
                       "'apt update && apt install -y gcc g++ make curl git build-essential'", shell=True)
    else:
        print(f"Using existing Docker container: {container_name}")

# Create and install tools in Docker container
check_and_create_docker_container()

# === Execute builds in Docker container ===
for build_type in selected_types:
    for target in selected_targets:
        if platform == "linux":
            cmd = f"docker exec {container_name} bash -c 'g++ {build_map[target](build_type)}'"
        else:
            cmd = build_map[target](build_type)
        
        print(f"[{build_type.upper()}] Building {target}")
        ret = os.system(cmd)
        
        if ret != 0:
            print(f"Build failed for {target} ({build_type})")
            sys.exit(1)
