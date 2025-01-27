// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.


#ifndef ___EXPORT_API___

#if defined(_WIN32) || defined(_WIN64)
  #if defined(_MSC_VER)
    #define EXPORT_API __declspec(dllexport)
  #else
    #define EXPORT_API
  #endif
#else
  #define EXPORT_API __attribute__((visibility("default")))
#endif

// This library was coded for the Wyland/Kokuyo C/C++ project.
// 
// The Wyland/Kokuyo's code has a C interface, used in the Wyland's C# code.




#endif // ___EXPORT_API___