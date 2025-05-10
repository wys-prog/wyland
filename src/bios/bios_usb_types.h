#ifndef ___WYLAND_BIOS_USB_TYPES___
#define ___WYLAND_BIOS_USB_TYPES___

#include <stdint.h>

// USB data structures
typedef uint8_t USBByte;
typedef struct {
  USBByte *Bytes;
  uint64_t Len;
} USBPacket;

#endif // ? ___WYLAND_BIOS_USB_TYPES___