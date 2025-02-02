#ifndef ___STD_KOKUYO_C___
#define ___STD_KOKUYO_C___

#include <cstdint>
#include <ctime>

/* Time encoding */
#define _std_kokuyo_encode_time(year, month, day, hour, minute, second, buff)  \
  buff = 0;                                                                    \
  buff |= (static_cast<uint64_t>(year) & 0xFFFF) << 48;                        \
  buff |= (static_cast<uint64_t>(month) & 0xFF) << 40;                         \
  buff |= (static_cast<uint64_t>(day) & 0xFF) << 32;                           \
  buff |= (static_cast<uint64_t>(hour) & 0xFF) << 24;                          \
  buff |= (static_cast<uint64_t>(minute) & 0xFF) << 16;                        \
  buff |= (static_cast<uint64_t>(second) & 0xFF);

/* Get time encoded in 64 bits */
#define _std_kokuyo_get_encoded_time(time, buff)                               \
  _std_kokuyo_encode_time(time->tm_year + 1900, time->tm_mon + 1,              \
                          time->tm_mday, time->tm_hour, time->tm_min,          \
                          time->tm_sec, buff)

/* System information */
#ifdef _WIN32
#define _std_kokuyo_os_string "Windows"
#define _std_kokuyo_os_code 0x00

#include <windows.h>

#define _std_kokuyo_cpu_count                                                  \
  (GetSystemInfo(&(SYSTEM_INFO){0}), GetSystemInfo(&(SYSTEM_INFO){0}),         \
   (SYSTEM_INFO){0}.dwNumberOfProcessors)
#define _std_kokuyo_ram_total                                                  \
  ((GlobalMemoryStatusEx(&(MEMORYSTATUSEX){sizeof(MEMORYSTATUSEX)}),           \
    (MEMORYSTATUSEX){sizeof(MEMORYSTATUSEX)}.ullTotalPhys) /                   \
   (1024 * 1024))

#elif defined(__linux__)
#define _std_kokuyo_os_string "Linux"
#define _std_kokuyo_os_code 0x01

#include <sys/sysinfo.h>
#include <unistd.h>

#define _std_kokuyo_cpu_count sysconf(_SC_NPROCESSORS_ONLN)
#define _std_kokuyo_ram_total                                                  \
  (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / (1024 * 1024))

#elif defined(__APPLE__)
#define _std_kokuyo_os_string "Apple"
#define _std_kokuyo_os_code 0x02

#include <sys/sysctl.h>
#define _std_kokuyo_cpu_count                                                  \
  ({                                                                           \
    int count;                                                                 \
    size_t size = sizeof(count);                                               \
    sysctlbyname("hw.ncpu", &count, &size, NULL, 0) == 0 ? count : -1;         \
  })
#define _std_kokuyo_ram_total                                                  \
  ({                                                                           \
    int64_t mem;                                                               \
    size_t size = sizeof(mem);                                                 \
    sysctlbyname("hw.memsize", &mem, &size, NULL, 0) == 0                      \
        ? mem / (1024 * 1024)                                                  \
        : -1;                                                                  \
  })

#else
#define _std_kokuyo_os_string "Unknown"
#define _std_kokuyo_os_code 0xFF
#define _std_kokuyo_cpu_count 0
#define _std_kokuyo_ram_total 0

#endif

#endif // ___STD_KOKUYO_C___
