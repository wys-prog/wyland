#include <boost/container/flat_map.hpp>

#include "libcallc.hpp"
#include "wmmbase.hpp"
#include "wmutiles.hpp"

#include "def/wiodef.hpp"
#include "def/usbdef.hpp"
#include "interfaces/interface.hpp"
#include "bios/bios.hpp"

WYLAND_BEGIN

namespace RuntimeBuiltIns {

  class WylandBuiltInCache {
  private:
    boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> linked_funcs;
    std::vector<libcallc::DynamicLibrary> libraries;
    IWylandGraphicsModule *GraphicsModulePtr = nullptr;
    WylandMMIOModule      *MMIOModule1Ptr = nullptr;
    WylandMMIOModule      *MMIOModule2Ptr = nullptr;
    WylandMMIOModule      *DiskModulePtr = nullptr;
    BIOS                  *BiosPtr = nullptr;

    std::vector<WylandMMIOModule*> SecurityMMIOPointers;
  
    std::vector<wuint> WylandDiskModuleBuffer;
    std::array<wuint, WUINTS_PER_BLOCK> ReadBlockBuffer = {};
    size_t ReadBlockIndex = 0;
    size_t ReadBlockSize = 0;

    std::vector<DynamicLibraryHandle> IExternalGraphicsModuleHandles;
  
    std::vector<DynamicLibraryHandle> WylandMMIOModuleHandles;

    std::vector<USBDrive*> USBDrivePointersCache;
    std::vector<USBDrive*> USBDevices;

  public:

    IWylandGraphicsModule *GetGraphicsModulePointer() { return GraphicsModulePtr; }
    WylandMMIOModule *GetMMIOModule1Pointer() { return MMIOModule1Ptr; }
    WylandMMIOModule *GetMMIOModule2Pointer() { return MMIOModule2Ptr; }
    WylandMMIOModule *GetDiskModulePointer() { return DiskModulePtr; }
    BIOS *GetBiosPointer() { return BiosPtr; }
    std::vector<WylandMMIOModule*> &GetSecurityMMIOPointers() { return SecurityMMIOPointers; }
    std::vector<wuint>& GetWylandDiskModuleBuffer() { return WylandDiskModuleBuffer; }
    std::array<wuint, WUINTS_PER_BLOCK>& GetReadBlockBuffer() { return ReadBlockBuffer; }
    std::vector<DynamicLibraryHandle>& GetIExternalGraphicsModuleHandles() { return IExternalGraphicsModuleHandles; }
    std::vector<DynamicLibraryHandle>& GetWylandMMIOModuleHandles() { return WylandMMIOModuleHandles; }
    std::vector<USBDrive*>& GetUSBDrivePointersCache() { return USBDrivePointersCache; }
    std::vector<USBDrive*>& GetUSBDevices() { return USBDevices; }
    boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType>& GetLinkedFuncs() { return linked_funcs; }
    std::vector<libcallc::DynamicLibrary>& GetLibraries() { return libraries; }

    void DestroyResources() {
      static bool can_destroy_memory(true);
      static bool can_destroy_gmptr(true);
      static bool can_destroy_mmio1(true);
      static bool can_destroy_mmio2(true);
      static bool can_destroy_bios(true);

      destroy(SecurityMMIOPointers);
      destroy(linked_funcs);
      destroy(libraries);
      destroy(IExternalGraphicsModuleHandles);
      destroy(WylandMMIOModuleHandles);
      destroy(WylandDiskModuleBuffer);
      ReadBlockBuffer.fill(0x00000000);
      if (memory) delete[] memory;
      if (GraphicsModulePtr) delete  GraphicsModulePtr;
      if (MMIOModule1Ptr) delete  MMIOModule1Ptr;
      if (MMIOModule2Ptr) delete  MMIOModule2Ptr;
      if (DiskModulePtr) delete  DiskModulePtr;
      if (BiosPtr) delete  BiosPtr;
      memory = nullptr;
      GraphicsModulePtr = nullptr;
      MMIOModule1Ptr = nullptr;
      MMIOModule2Ptr = nullptr;
      DiskModulePtr = nullptr;
      BiosPtr = nullptr;
      ReadBlockIndex = 0;
      
      destroy(USBDevices);
      destroy(USBDrivePointersCache);
      can_destroy_memory = false;
      can_destroy_gmptr  = false;
      can_destroy_mmio1  = false;
      can_destroy_mmio2  = false;
      can_destroy_bios   = false;
    }
  };

}

WYLAND_END

extern "C" {
  static wylma::wyland::RuntimeBuiltIns::WylandBuiltInCache g_WylandBuiltInCache;

  wylma::wyland::IWylandGraphicsModule* GetGraphicsModulePointer() {
    return g_WylandBuiltInCache.GetGraphicsModulePointer();
  }

  wylma::wyland::WylandMMIOModule* GetMMIOModule1Pointer() {
    return g_WylandBuiltInCache.GetMMIOModule1Pointer();
  }

  wylma::wyland::WylandMMIOModule* GetMMIOModule2Pointer() {
    return g_WylandBuiltInCache.GetMMIOModule2Pointer();
  }

  wylma::wyland::WylandMMIOModule* GetDiskModulePointer() {
    return g_WylandBuiltInCache.GetDiskModulePointer();
  }

  wylma::wyland::BIOS* GetBiosPointer() {
    return g_WylandBuiltInCache.GetBiosPointer();
  }

  std::vector<wylma::wyland::WylandMMIOModule*>* GetSecurityMMIOPointers() {
    return &g_WylandBuiltInCache.GetSecurityMMIOPointers();
  }

  std::vector<wuint>* GetWylandDiskModuleBuffer() {
    return &g_WylandBuiltInCache.GetWylandDiskModuleBuffer();
  }

  std::array<wuint, WUINTS_PER_BLOCK>* GetReadBlockBuffer() {
    return &g_WylandBuiltInCache.GetReadBlockBuffer();
  }

  std::vector<DynamicLibraryHandle>* GetIExternalGraphicsModuleHandles() {
    return &g_WylandBuiltInCache.GetIExternalGraphicsModuleHandles();
  }

  std::vector<DynamicLibraryHandle>* GetWylandMMIOModuleHandles() {
    return &g_WylandBuiltInCache.GetWylandMMIOModuleHandles();
  }

  std::vector<wylma::wyland::USBDrive*>* GetUSBDrivePointersCache() {
    return &g_WylandBuiltInCache.GetUSBDrivePointersCache();
  }

  std::vector<wylma::wyland::USBDrive*>* GetUSBDevices() {
    return &g_WylandBuiltInCache.GetUSBDevices();
  }

  boost::container::flat_map<uint32_t, wylma::wyland::libcallc::DynamicLibrary::FunctionType>* GetLinkedFuncs() {
    return &g_WylandBuiltInCache.GetLinkedFuncs();
  }

  std::vector<wylma::wyland::libcallc::DynamicLibrary>* GetLibraries() {
    return &g_WylandBuiltInCache.GetLibraries();
  }


}