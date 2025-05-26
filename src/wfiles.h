#ifndef ___WYLAND_FILES_H___
#define ___WYLAND_FILES_H___

#ifdef _WIN32
#include <windows.h>
#endif 

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "targets.h"
#include "wyland.h"

#include "sock2.h"

/* Why did i started to code this fucking project ? */

typedef struct {
  uint8_t  certificat[3];
  warch_t  target;
  uint32_t version;
  uint64_t code;
  uint64_t data;
  uint64_t lib;
} wheader_t;

typedef struct {
  uint8_t array[sizeof(wheader_t)];
} wblock;

inline wheader_t wyland_files_make_header(const wblock *block) {
  wheader_t header = {};

  memcpy(header.certificat, block->array, sizeof(header.certificat));

  memcpy(&header.target, block->array + sizeof(header.certificat), sizeof(header.target));
  header.target = swap16(header.target); // Convert back

  memcpy(&header.version, block->array + sizeof(header.certificat) + sizeof(header.target), sizeof(header.version));
  header.version = swap32(header.version); // Convert back

  memcpy(&header.code, block->array + sizeof(header.certificat) + sizeof(header.target) + sizeof(header.version), sizeof(header.code));
  header.code = swap64(header.code); // Convert back

  memcpy(&header.data, block->array + sizeof(header.certificat) + sizeof(header.target) + sizeof(header.version) + sizeof(header.code), sizeof(header.data));
  header.data = swap64(header.data); // Convert back

  memcpy(&header.lib, block->array + sizeof(header.certificat) + sizeof(header.target) + sizeof(header.version) + sizeof(header.code) + sizeof(header.data), sizeof(header.lib));
  header.lib = swap64(header.lib); // Convert back

  return header;
}

inline const char *wyland_files_header_fmt(const wheader_t *header) {
  static char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "Certificate:\t%c%c%c\n"
           "Target:\t\t%u (%s)\n"
           "Version:\t%u (%s)\n"
           "Code Address:\t0x%016llX\n"
           "Data Address:\t0x%016llX\n"
           "Lib Address:\t0x%016llX\n",
           header->certificat[0], header->certificat[1], header->certificat[2],
           header->target, nameof(header->target), 
           header->version, (header->version <= WYLAND_VERSION_UINT32 ? "supported" : "unsopported"),
           header->code,
           header->data,
           header->lib);
  return buffer;
}

inline const char *wyland_files_fmt_header_spec(const wblock *block, char del = ',') {
  static char buffer[256];

  snprintf(buffer, sizeof(buffer), 
    "{%c%c%c}(certif.)%c{%u.%u}(target)%c{%u.%u.%u.%u}(version)%c"
    "{%u.%u.%u.%u.%u.%u.%u.%u}(code section)%c"
    "{%u.%u.%u.%u.%u.%u.%u.%u}(data section)%c"
    "{%u.%u.%u.%u.%u.%u.%u.%u}(libs section)", 
    block->array[0], block->array[1], block->array[2], del, 
    block->array[3], block->array[4], del, 
    block->array[5], block->array[6], block->array[7], block->array[8], del,
    /* addresses */
    block->array[9], block->array[10], block->array[11], block->array[12], 
    block->array[13], block->array[14], block->array[15], block->array[16], del,
    // data
    block->array[17], block->array[18], block->array[19], block->array[20], 
    block->array[21], block->array[22], block->array[23], block->array[24], del,
    // libs
    block->array[25], block->array[26], block->array[27], block->array[28],
    block->array[29], block->array[30], block->array[31], block->array[32]
  );

  return buffer;
}

inline int wyland_files_parse(const wheader_t *header, int16_t target, int32_t version) {
  if (memcmp(header->certificat, "wlf", 3) != 0) {
    fprintf(stderr, "[e]: Invalid certificate.\n");
    return 0;
  }
  
  if (header->target != target) {
    fprintf(stderr, 
      "[w]: Mismatched target. File was built for target %d, but running on target %d. "
      "This may cause runtime errors.\n", 
      header->target, target
    );
  }

  if (header->version > (uint32_t)version) {
    fprintf(stderr, 
      "[e]: File version (%d) is newer than supported version (%d). Cannot proceed.\n",
      header->version, version
    );
    return 0;
  }

  if (header->version < (uint32_t)version) {
    fprintf(stderr, 
      "[w]: File version (%d) is older than the supported version (%d). Compatibility issues may occur.\n",
      header->version, version
    );
  }

  return 1;
}

inline wblock wyland_files_header_to_block(const wheader_t *header) {
  wblock block = {0};

  memcpy(block.array, header->certificat, sizeof(header->certificat));

  uint16_t target_be = swap16(header->target);
  memcpy(block.array + sizeof(header->certificat), &target_be, sizeof(target_be));

  uint32_t version_be = swap32(header->version);
  memcpy(block.array + sizeof(header->certificat) + sizeof(target_be), &version_be, sizeof(version_be));

  uint64_t code_be = swap64(header->code);
  memcpy(block.array + sizeof(header->certificat) + sizeof(target_be) + sizeof(version_be), &code_be, sizeof(code_be));

  uint64_t data_be = swap64(header->data);
  memcpy(block.array + sizeof(header->certificat) + sizeof(target_be) + sizeof(version_be) + sizeof(code_be), &data_be, sizeof(data_be));

  uint64_t lib_be = swap64(header->lib);
  memcpy(block.array + sizeof(header->certificat) + sizeof(target_be) + sizeof(version_be) + sizeof(code_be) + sizeof(data_be), &lib_be, sizeof(lib_be));

  return block;
}



inline wheader_t wyland_files_basic_header() {
  wheader_t header = {};

  header.certificat[0] = 'w';
  header.certificat[1] = 'l';
  header.certificat[2] = 'f';
  
  header.code = 0x00000000000000FF;
  header.data = 0x0000000000FFFFFF;
  header.lib  = 0x0000000010AAEEEE;
  header.target = warch64;
  header.version = WYLAND_VERSION_UINT32;

  return header;
};

#endif // ___WYLAND_FILES_H___