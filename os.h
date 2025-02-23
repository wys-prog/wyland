#ifndef PLATFORM_NAMES_H
#define PLATFORM_NAMES_H

// x86 Windows (Win32) → "Ötüken"
#if defined(_WIN32) && !defined(_WIN64)
    #define PLATFORM_NAME "Ötüken" // Historical center of the Göktürk Empire

// x64 Windows (Win64) → "Börü"
#elif defined(_WIN64)
    #define PLATFORM_NAME "Börü" // The dominant and powerful wolf

// ARM Windows (ARM Win) → "Orkhon"
#elif defined(_WIN32) && defined(__ARM_ARCH)
    #define PLATFORM_NAME "Orkhon" // A new era, like the Orkhon inscriptions

// ARM64 Windows (ARM64 Win) → "Kül Tegin"
#elif defined(_WIN64) && defined(__aarch64__)
    #define PLATFORM_NAME "Kül Tegin" // Fast and powerful, like the legendary warrior

// x86 Linux (Linux x86) → "Altay"
#elif defined(__linux__) && defined(__i386__)
    #define PLATFORM_NAME "Altay" // Ancient but resilient like the Altay Mountains

// x64 Linux (Linux x64) → "Balgasun"
#elif defined(__linux__) && defined(__x86_64__)
    #define PLATFORM_NAME "Balgasun" // Secure and stable like the fortified city

// ARM Linux (Linux ARM) → "Talas"
#elif defined(__linux__) && defined(__arm__)
    #define PLATFORM_NAME "Talas" // Transformative like the Battle of Talas

// ARM64 Linux (Linux ARM64) → "Sayan"
#elif defined(__linux__) && defined(__aarch64__)
    #define PLATFORM_NAME "Sayan" // Expanding globally like the Sayan Mountains

// x64 macOS (Mac Intel) → "Issyk"
#elif defined(__APPLE__) && defined(__x86_64__)
    #define PLATFORM_NAME "Issyk" // Elegant and refined like the Issyk-Kul Lake

// ARM64 macOS (Mac M1/M2/M3/M4) → "Ergenekon"
#elif defined(__APPLE__) && defined(__aarch64__)
    #define PLATFORM_NAME "Ergenekon" // A powerful rebirth, like the mythological valley

// RISC-V Linux (Linux RISC-V) → "Tengri"
#elif defined(__linux__) && defined(__riscv)
    #define PLATFORM_NAME "Tengri" // Boundless potential, like the sky god Tengri
#else 
// Default to unknown if no platform is detected
    #define PLATFORM_NAME "Anadolu"
#endif // End of platform detection

#endif // PLATFORM_NAMES_H
