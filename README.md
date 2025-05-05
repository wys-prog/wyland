# Wyland 🚀⚡💻

## Introduction
Wyland is a virtual machine that uses a custom instruction set.  
The virtual machine can load programs (using `-run` if a header is present in the disk, or `-run-raw` if you want to run a raw file), and interact with the system.  
By default, it cannot interact with the system, but you can enable interactions by loading libraries (in the `lib` section of your disk file), written with a C interface (see how libraries work), or by loading an MMIO Module (which must implement the correct MMIO-C interface — see below).

## More Details
### MMIO Modules
Let's get into a bit more detail about MMIO modules.

The first MMIO module is the **GraphicsModule**, which inherits from the `WylandMMIOModule` class.  
There are multiple types of GraphicsModules in Wyland. The default one is `WylandGraphicsModule`, which does basically nothing.  
The second one, `IWylandExternalGraphicsModule`, allows users to dynamically load a `GraphicsModule` (you can specify it with `-gm <FILE>` or `-GraphicsModule <FILE>`).  
We're also working on a built-in GraphicsModule, and a customizable built-in GraphicsModule.

Next, there are `MMIOModule1` and `MMIOModule2`, which are two 'free' modules.  
Users can define them using `-m1` or `-m2`.  
If the user defines an external MMIO module, the `IWylandExternalMMIOModule` class will be used. Otherwise, the default `WylandMMIOModule` class will be used.

### Memory
The virtual machine's memory is a simple dynamic array declared in the `wmmbase.hpp` (Wyland Machine Memory Base) file.  
Memory is allocated **only** after the core pointer is successfully initialized.  
If an error occurs during initialization, memory is **not** allocated.  
If memory was allocated but an error happens later, the memory segment is cleared by the `wyland_exit(int)` function.

### Wyland C Runtime Library
There is a C runtime library that you can include in your project, but you must compile it first.  
This runtime is used both by the virtual machine itself and by your APIs (whether it's an MMIO Module, a Graphics Module, or a linked function library).

#### Compilation
```bash
c++-compiler wyland-runtime/bindings.cpp -o wyland-runtime/bin/bindings.o
c-compiler wyland-runtime/bin/bindings.o wyland-runtime/wylrt.c -o wyland-runtime/bin/runtime.o
c-compiler project.c wyland-runtime/bin/bindings.o wyland-runtime/bin/runtime.o -o output
```

## Running
Now let's see how to actually run it.

The default binaries expect a `./bin/` folder, where they can find `runtime.o`, and sometimes `bindings.o`.  
**Note:** This might change in the future, and the number of required files could increase.

You have two options:
- Include Wyland in your project and create a `bin/` folder (or copy an existing one).
- Or add your VM to your system path / create an alias.

I haven't tested path handling on Windows yet, but on macOS, creating an alias works pretty well!

Also, the runtime's (`wylrt.c`) version might be different from the kernel's (`wyland.cpp`) version.  
If the runtime or kernel is too old, it might cause errors or refuse to work.

---

This documentation is not finished yet, and I know it's really bad — I'm extremely sorry about that 😭.  
If you need more information, feel free to contact me:

- Discord: `wyslte`
- Email: `wysltek@gmail.com`

**Note:**  
If you see `JS` mentioned anywhere in the project — it's absolutely not JavaScript!  
It was an inside joke with a friend who said, "Oh! Java Shit!" (because my project is inspired by the JVM).

### Details on running
Here are all the commands available in Wyland:

#### General Commands
- `--v`, `--version`: Prints the version of Wyland.
- `--n`, `--name`: Prints the name of Wyland.
- `--b`, `--build`: Prints the build name of Wyland.
- `--target`: Prints the default target.
- `--target-info`: Prints information about all available targets.
- `--info`, `--i`: Prints detailed information about Wyland, including runtime, system, and compiler details.

#### Disk and File Commands
- `-check <file>`: Checks the specified disk file and prints its header information.
  - `--raw <file>`: Prints the raw content of the disk file.
  - `--spec <file>`: Prints the header specification of the disk file.
  - `--libs <file>`: Prints the libraries linked in the disk file.
- `-make-disk <disk> [options]`: Creates a new disk file with the specified name.
  - Options:
    - `-version <x>`: Sets the version of the disk.
    - `-target <x>`: Sets the target of the disk.
    - `-data <x>`: Sets the data address of the disk.
    - `-code <x>`: Sets the code address of the disk.
    - `-lib <x>`: Sets the library address of the disk.

#### Running and Debugging
- `-run <files...>`: Runs the specified disk files with a valid header.
- `-run-raw <files...>`: Runs the specified raw files without requiring a header.
- `-debug <files...>`: Runs the specified disk files in debug mode.

#### Target Configuration
- `-target <x>`: Sets the target to `<x>`.
- `-max <x>`, `-max-cycles <x>`: Sets the maximum number of cycles (only works in debug mode).
- `-auto`: Automatically detects the target from the disk header.

#### Library and Module Configuration
- `-GraphicsModule <file>`, `-gm <file>`: Specifies the path to the Graphics Module.
- `-module1 <file>`, `-m1 <file>`: Specifies the path to the first MMIO module.
- `-module2 <file>`, `-m2 <file>`: Specifies the path to the second MMIO module.
- `-fmt-libs-false`, `-nofmtlibs`: Disables library name formatting.

#### Additional Commands
- `-libsof <file>`: Lists all libraries and their functions in the specified disk file.
- `-look <file>`: Alias for `-check`.
- `-no-init-out`: Suppresses initialization output.
- `-memory:<size>`: Allocates the specified memory size for the virtual machine.
- `-compile <files...>` or `-c <files...>`: Compiles the specified assembly files into a binary.
- `-update`: Updates Wyland using the updater module.

---

**Note:**  
- The `-run` command expects a disk file with a valid header. If the header is invalid, the program will terminate with an error.
- The `-run-raw` command allows running raw files directly, but it requires manual memory allocation and module loading.

Have fun exploring Wyland! 😈
