#ifndef ___WYLAND_FILES_H___
#define ___WYLAND_FILES_H___

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "targets.h"
#include "wyland.h"

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
  if (strcmp((char*)header->certificat, "wlf") != 0) {
    fprintf(stderr, "[e]: Invalid certificate.\n");
    return 0;
  }

  if (header->target != target) {
    fprintf(stderr, 
      "[e]: Mismatched target. File was built for target %d, but running on target %d. "
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

wblock wyland_files_header_to_block(const wheader_t *header) {
  wblock block = {};

  memcpy(block.array, header->certificat, sizeof(header->certificat));
  memcpy(block.array + sizeof(header->certificat), &header->target, sizeof(header->target));
  memcpy(block.array + sizeof(header->certificat) + sizeof(header->target), &header->version, sizeof(header->version));
  memcpy(block.array + sizeof(header->certificat) + sizeof(header->target) + sizeof(header->version), &header->code, sizeof(header->code));
  memcpy(block.array + sizeof(header->certificat) + sizeof(header->target) + sizeof(header->version) + sizeof(header->code), &header->data, sizeof(header->data));
  memcpy(block.array + sizeof(header->certificat) + sizeof(header->target) + sizeof(header->version) + sizeof(header->code) + sizeof(header->data), &header->lib, sizeof(header->lib));

  return block;
}

wheader_t wyland_files_basic_header() {
  wheader_t header = {};

  header.certificat[0] = 'w';
  header.certificat[1] = 'l';
  header.certificat[2] = 'f';
  
  header.code = 0x00000000000000FF;
  header.data = 0x0000000000FFFFFF;
  header.lib  = 0x0000000010AAEEEE;
  header.target = wtarg64;
  header.version = WYLAND_VERSION_UINT32;

  return header;
};

#endif // ___WYLAND_FILES_H___