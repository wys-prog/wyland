#pragma once

#include <boost/container/flat_map.hpp>

#include "libcallc.hpp"
#include "wmmbase.hpp"
#include "wmutiles.hpp"

#include "def/wiodef.hpp"
#include "def/usbdef.hpp"
#include "interfaces/interface.hpp"
#include "bios/bios.hpp"
extern "C" {
  wylma::wyland::IWylandGraphicsModule* GetGraphicsModulePointer();

  wylma::wyland::WylandMMIOModule* GetMMIOModule1Pointer();

  wylma::wyland::WylandMMIOModule* GetMMIOModule2Pointer();

  wylma::wyland::WylandMMIOModule* GetDiskModulePointer();

  wylma::wyland::BIOS* GetBiosPointer();

  std::vector<wylma::wyland::WylandMMIOModule*>* GetSecurityMMIOPointers();

  std::vector<wuint>* GetWylandDiskModuleBuffer();

  std::array<wuint, WUINTS_PER_BLOCK>* GetReadBlockBuffer();

  std::vector<DynamicLibraryHandle>* GetIExternalGraphicsModuleHandles();

  std::vector<DynamicLibraryHandle>* GetWylandMMIOModuleHandles();

  std::vector<wylma::wyland::USBDrive*>* GetUSBDrivePointersCache();

  std::vector<wylma::wyland::USBDrive*>* GetUSBDevices();

  boost::container::flat_map<uint32_t, wylma::wyland::libcallc::DynamicLibrary::FunctionType>* GetLinkedFuncs();

  std::vector<wylma::wyland::libcallc::DynamicLibrary>* GetLibraries();
}