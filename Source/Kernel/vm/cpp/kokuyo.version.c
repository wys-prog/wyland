// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

#ifndef ___KOKUYO_VERSION___

/*
 * Core Versioning
 * This defines the version of the core system for Kokuyo.
 * Currently, version 1.0 is used for the Wyland project.
 */
#define CORE_VERSION              1.0            // The version number of the core system.

#define CORE_HALTER               0xFF           // Special halting code for system termination or emergency stop.

// Core operational modes
// Defines the operational mode of the system and how memory is handled during runtime.
#define CORE_MODE                 "dynamic-vector-based"  // The system uses dynamic vectors for memory and data handling.
#define CORE_MEMORY_MODE          "dynamic-vector-based"  // Describes the memory management scheme used for dynamic allocation.

// Hardware support flags (16-bit, 32-bit, 64-bit)
// These flags indicate whether support is enabled for specific bit-widths of the system.
#define CORE_SUPPORT_DISK         0              // Indicates if disk-based storage is supported (0 = Disabled, 1 = Enabled).
#define CORE_SUPPORT_16           0              // Support for 16-bit architecture (0 = Not supported).
#define CORE_SUPPORT_32           0              // Support for 32-bit architecture (0 = Not supported).
#define CORE_SUPPORT_64           1              // Support for 64-bit architecture (1 = Supported, 0 = Not supported).

// Stack management
// The following defines set the maximum size for system stacks and call stacks.
#define CORE_STACK_MAX            4096           // Maximum stack size in bytes for the system.
#define CORE_CALLSTACK_MAX        1024           // Maximum call stack depth, defines how many function calls can be nested.
#define CORE_CALLSTACK_IS_STACK   0              // Determines if the call stack is implemented using a stack data structure (1 = Yes, 0 = No).

// Core naming and register configuration
// The core name and number of registers are defined here for identification and processing purposes.
#define CORE_NAME                 "kokuyo/Wyland - 1.0"  // Name of the core system for this specific project build.
#define CORE_REGISTERS_COUNT      32                     // The number of registers available in the core system (standard for many processors).

#endif // ___KOKUYO_VERSION___
