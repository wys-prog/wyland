#ifndef ___STD_KOKUYO_C___
#define ___STD_KOKUYO_C___

/* _std_kokuyo_encode_time(year, month, day, hour, minute, second, buff)
 *
 * This macro will encode the given time in a uint64_t.
 * Use this macro in another type (with a fewer size) can
 * make unexcepted results.
 */
#define _std_kokuyo_encode_time(year, month, day, hour, minute, second, buff)  \
  buff = 0;                                                                    \
  buff |= (static_cast<uint64_t>(year) & 0xFFFF) << 48;                        \
  buff |= (static_cast<uint64_t>(month) & 0xFF) << 40;                         \
  buff |= (static_cast<uint64_t>(day) & 0xFF) << 32;                           \
  buff |= (static_cast<uint64_t>(hour) & 0xFF) << 24;                          \
  buff |= (static_cast<uint64_t>(minute) & 0xFF) << 16;                        \
  buff |= (static_cast<uint64_t>(second) & 0xFF);


/* These macros are defined to get informations about the system. */
#ifdef _WIN32
# define _std_kokuyo_os_string "Windows"  /* Get a string of operating system’s name */
# define _std_kokuyo_os_code    0x00      /* Get a custom code */
#elif defined(__linux__)
# define _std_kokuyo_os_string "Linux"    /* Get a string of operating system’s name */
# define _std_kokuyo_os_code    0x01      /* Get a custom code */
#elif defined(__APPLE__)
# define _std_kokuyo_os_string "Apple"    /* Get a string of operating system’s name */
# define _std_kokuyo_os_code    0x02      /* Get a custom code */
#else 
# define _std_kokuyo_os_string "Unknown"  /* Get a string of operating system’s name */
# define _std_kokuyo_os_code    0xFF      /* Get a custom code */
#endif



#endif // ___STD_KOKUYO_C___