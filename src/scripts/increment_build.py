import sys
from pathlib import Path

infile = Path(sys.argv[1])
outfile = Path(sys.argv[2])

major = 1
minor = 0

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

version_str = f"{major}.{minor:02}.{build:07}"

with open(outfile, 'w') as f:
    f.write(f'#ifndef ___WYLAND_BUILD_SERVICE___\n#define ___WYLAND_BUILD_SERVICE___\n#define WYLAND_VERSION "{version_str}"\n#endif')
