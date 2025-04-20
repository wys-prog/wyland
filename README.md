# Wyland 🚀⚡💻

## Intruduction
Wyland is a virtual machine, which uses a custom instruction set.
The virtual machine can load programs (with `-run` if a header is present in the disk, or with `-run-raw` if you would like to run a raw file), and interacts with the system. By default, you can't, but by loading libraries (in the `lib` section of your disk file), written with a C interface (see how libraries works), or by loading a MMIO Module (which presents a correct MMIO-C interface, see how).

## Specific introduction
### MMIO Modules
So here, we'll enter in details.
Let's see first MMIO modules

First one is the GraphicsModule, which herits the `WylandMMIOModule` class.
And there's multiple GraphicsModules in the virtual machine. First one is the default class, `WylandGraphicsModule`, which does nothing. The second one, which is `IWylandExternalGraphicsModule` lets user to load dynamicly a `GraphicsModule` (can specified with `-gm <FILE>` or `-GraphicsModule <FILE>`). Finally, we're working on built-in GraphicsModule, and a built-in modulable GraphicsModule.
Second one, is `MMIOModule1` and `MMIOModule2`, which are two 'free' modules. User can define them, by using `-m1` or `-m2`. 
In the case where user defines an external MMIOModule, the used class will be `IWylandExternalMMIOModule`. In other cases, it will be `WylandMMIOModule`.

### Memory
Memory of virtual machine is a simple dynamic array declared in the `wmmbase.hpp` (Wyland Machine Memory Base) file.
Memory is allocated **only** when first initialization of the core pointer is okay. In error case, memory isn't allocated.
In the case where memory is allocated, but error(s) occure, memory segment is cleared by the `wyland_exit(int)` function.

### Wyland C Runtime Library
There's a C Runtime that you can include, but you must compile before. This runtime is used either by the virtual machine, and your API (whatever can be, a MMIOModule, a GraphicsModule, or a linked function Library (libraries to make is simpler)).

#### Compilation
```bash
c++-compiler wyland-runtime/bindings.cpp -o wyland-runtime/bin/bindings.o
c-compiler wyland-runtime/bin/bindings.o wyland-runtime/wylrt.c -o wyland-runtime/bin/runtime.o
c-compiler project.c wyland-runtime/bin/bindings.o wyland-runtime/bin/runtime.o -o output
```
