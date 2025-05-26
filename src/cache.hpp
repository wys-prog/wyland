#pragma once

#include <boost/container/flat_map.hpp>

#include "libcallc.hpp"
#include "wmmbase.hpp"
#include "icache.hpp"

WYLAND_BEGIN

class WylandMMIOModule;
class IWylandGraphicsModule;
class IWylandDiskModule;
class BIOS;
class USBDrive;

class cache {
public:
  static boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> linked_funcs;
  static std::vector<libcallc::DynamicLibrary> libraries;
  static IWylandGraphicsModule *GraphicsModulePtr;
  static WylandMMIOModule      *MMIOModule1Ptr;
  static WylandMMIOModule      *MMIOModule2Ptr;
  static WylandMMIOModule      *DiskModulePtr;
  static BIOS                  *BiosPtr;

  static std::vector<WylandMMIOModule*> SecurityMMIOPointers;
  
  static std::vector<wuint> WylandDiskModuleBuffer;
  static std::array<wuint, WUINTS_PER_BLOCK> ReadBlockBuffer;
  static size_t ReadBlockIndex;
  static size_t ReadBlockSize;

  static std::vector<DynamicLibraryHandle> IExternalGraphicsModuleHandles;
  
  static std::vector<DynamicLibraryHandle> WylandMMIOModuleHandles;

  static std::vector<USBDrive*> USBDrivePointersCache;
  static std::vector<USBDrive*> USBDevices;

  static void init_cache() {
    GraphicsModulePtr = GetGraphicsModulePointer();
    MMIOModule1Ptr = GetMMIOModule1Pointer();
    MMIOModule2Ptr = GetMMIOModule2Pointer();
    DiskModulePtr = GetDiskModulePointer();
    BiosPtr = GetBiosPointer();
    SecurityMMIOPointers = *GetSecurityMMIOPointers();
    WylandDiskModuleBuffer = *GetWylandDiskModuleBuffer();
    ReadBlockBuffer = *GetReadBlockBuffer();
    IExternalGraphicsModuleHandles = *GetIExternalGraphicsModuleHandles();
    WylandMMIOModuleHandles = *GetWylandMMIOModuleHandles();
    USBDrivePointersCache = *GetUSBDrivePointersCache();
    USBDevices = *GetUSBDevices();
    linked_funcs = *GetLinkedFuncs();
    libraries = *GetLibraries();
  }
};


WYLAND_END