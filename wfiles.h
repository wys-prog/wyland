#ifndef ___WYLAND_FILES_H___
#define ___WYLAND_FILES_H___

#include <stdint.h>
#include <string.h>

typedef struct {
  uint8_t  certificat[3];
  uint16_t target;
  uint32_t version;
  uint64_t code;
  uint64_t data;
  uint64_t lib;
} wheader_t;

typedef struct {
  uint8_t array[sizeof(wheader_t)];
} wblock;

wheader_t wyland_files_make_header(const wblock *block) {
  wheader_t header = {};

  memcpy(header.certificat, block->array, sizeof(header.certificat));
  memcpy(&header.target, block->array + sizeof(header.certificat), sizeof(header.target));
  memcpy(&header.version, block->array + sizeof(header.certificat) + sizeof(header.target), sizeof(header.version));
  memcpy(&header.code, block->array + sizeof(header.certificat) + sizeof(header.target) + sizeof(header.version), sizeof(header.code));
  memcpy(&header.data, block->array + sizeof(header.certificat) + sizeof(header.target) + sizeof(header.version) + sizeof(header.code), sizeof(header.data));
  memcpy(&header.lib, block->array + sizeof(header.certificat) + sizeof(header.target) + sizeof(header.version) + sizeof(header.code) + sizeof(header.data), sizeof(header.lib));

  return header;
}

int wyland_files_parse(const wheader_t *header, int16_t target, int32_t version) {
  if (strcmp((char*)header->certificat, "wlf") != 0) return 0;

  /* Other checks later... */

  return 1;
}

#endif // ___WYLAND_FILES_H___