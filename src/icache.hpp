#pragma once

#include <boost/container/flat_map.hpp>

#include "libcallc.hpp"
#include "wmmbase.hpp"
#include "wmutiles.hpp"
/*
#include "def/wiodef.hpp"
#include "def/usbdef.hpp"
#include "interfaces/interface.hpp"
#include "bios/bios.hpp"
*/
WYLAND_BEGIN

class USBDevice;
class ExternalGraphicsModule;
class WylandMMIOModule;
class IWylandGraphicsModule;
class BIOS;
class USBDrive;

WYLAND_END

extern "C" {
  wylma::wyland::IWylandGraphicsModule* eGetGraphicsModulePointer();

  wylma::wyland::WylandMMIOModule* eGetMMIOModule1Pointer();

  wylma::wyland::WylandMMIOModule* eGetMMIOModule2Pointer();

  wylma::wyland::WylandMMIOModule* eGetDiskModulePointer();

  wylma::wyland::BIOS* eGetBiosPointer();

  std::vector<wylma::wyland::WylandMMIOModule*>* eGetSecurityMMIOPointers();

  std::vector<wuint>* eGetWylandDiskModuleBuffer();

  std::array<wuint, WUINTS_PER_BLOCK>* eGetReadBlockBuffer();

  std::vector<DynamicLibraryHandle>* eGetIExternalGraphicsModuleHandles();

  std::vector<DynamicLibraryHandle>* eGetWylandMMIOModuleHandles();

  std::vector<wylma::wyland::USBDrive*>* eGetUSBDrivePointersCache();

  std::vector<wylma::wyland::USBDrive*>* eGetUSBDevices();

  boost::container::flat_map<uint32_t, wylma::wyland::libcallc::DynamicLibrary::FunctionType>* eGetLinkedFuncs();

  std::vector<wylma::wyland::libcallc::DynamicLibrary>* eGetLibraries();
}

WYLAND_BEGIN

namespace IWylandCache {
  inline IWylandGraphicsModule* GetGraphicsModulePointer() {
    return eGetGraphicsModulePointer();
  }

  inline WylandMMIOModule* GetMMIOModule1Pointer() {
    return eGetMMIOModule1Pointer();
  }

  inline WylandMMIOModule* GetMMIOModule2Pointer() {
    return eGetMMIOModule2Pointer();
  }

  inline WylandMMIOModule* GetDiskModulePointer() {
    return eGetDiskModulePointer();
  }

  inline BIOS* GetBiosPointer() {
    return eGetBiosPointer();
  }

  inline std::vector<WylandMMIOModule*>* GetSecurityMMIOPointers() {
    return eGetSecurityMMIOPointers();
  }

  inline std::vector<wuint>* GetWylandDiskModuleBuffer() {
    return eGetWylandDiskModuleBuffer();
  }

  inline std::array<wuint, WUINTS_PER_BLOCK>* GetReadBlockBuffer() {
    return eGetReadBlockBuffer();
  }

  inline std::vector<DynamicLibraryHandle>* GetIExternalGraphicsModuleHandles() {
    return eGetIExternalGraphicsModuleHandles();
  }

  inline std::vector<DynamicLibraryHandle>* GetWylandMMIOModuleHandles() {
    return eGetWylandMMIOModuleHandles();
  }

  inline std::vector<USBDrive*>* GetUSBDrivePointersCache() {
    return eGetUSBDrivePointersCache();
  }

  inline std::vector<USBDrive*>* GetUSBDevices() {
    return eGetUSBDevices();
  }

  inline boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType>* GetLinkedFuncs() {
    return eGetLinkedFuncs();
  }

  inline std::vector<libcallc::DynamicLibrary>* GetLibraries() {
    return eGetLibraries();
  }
}

WYLAND_END
