#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

#include "../wyland-runtime/wylrt.hpp"
#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../disk.hpp"

WYLAND_BEGIN

namespace bios {
  typedef int64_t StreamPos;
  typedef uint64_t StreamSize;
  typedef int8_t  Byte;

  typedef struct {
    Byte Data[512];
    StreamPos Position;
  } Sector;

  typedef struct {
    Byte *Bytes;
    StreamSize Size;
    StreamPos  Position;
  } Bytes;

  class BiosDiskHandler {
    using InternFunc = void(BiosDiskHandler::*)();

  private:
    /*WylandMMIOModule *Disk;
  
    InternFunc funcs[6] = {

    };*/

  public:
    BiosDiskHandler(WylandMMIOModule *) /*: Disk(DiskPtr) */ {
      //Disk->name();
    }
    /*
    Sector ReadSector(StreamPos from) {}
    void WriteSector(const Sector &sector) {}
    Byte ReadByte(StreamPos from) {}
    void WriteByte(StreamPos to, Byte byte) {}
    Bytes ReadBytes(StreamPos from) {}
    void WriteBytes(StreamPos to, Bytes bytes) {}*/
  };

}

WYLAND_END