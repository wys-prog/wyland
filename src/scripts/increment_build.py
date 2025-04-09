import sys
from pathlib import Path

infile = Path(sys.argv[1])
outfile = Path(sys.argv[2])

major = 1
minor = 4

# read old counter or start at 0
if infile.exists():
    try:
        print("huh")
        build = int(infile.read_text().strip())
    except ValueError:
        build = 0
else:
    build = 0

build += 1
infile.write_text(str(build))

version_str = f"{major}.{minor:02}.{build:04}"
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
