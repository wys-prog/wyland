//
// Created by Hüseyin ÖZTÜRK on 28/05/2025.
//

#include <boost/container/flat_map.hpp>

#include "libcallc.hpp"
#include "wmmbase.hpp"
#include "icache.hpp"
#include "cache.hpp"

WYLAND_BEGIN

boost::container::flat_map<uint32_t, libcallc::DynamicLibrary::FunctionType> cache::linked_funcs;
std::vector<libcallc::DynamicLibrary> cache::libraries;
IWylandGraphicsModule* cache::GraphicsModulePtr = nullptr;
WylandMMIOModule* cache::MMIOModule1Ptr = nullptr;
WylandMMIOModule* cache::MMIOModule2Ptr = nullptr;
WylandMMIOModule* cache::DiskModulePtr = nullptr;
BIOS* cache::BiosPtr = nullptr;

std::vector<WylandMMIOModule*> cache::SecurityMMIOPointers;
std::vector<wuint> cache::WylandDiskModuleBuffer;
std::array<wuint, WUINTS_PER_BLOCK> cache::ReadBlockBuffer{};
size_t cache::ReadBlockIndex = 0;
size_t cache::ReadBlockSize = 0;

std::vector<DynamicLibraryHandle> cache::IExternalGraphicsModuleHandles;
std::vector<DynamicLibraryHandle> cache::WylandMMIOModuleHandles;

std::vector<USBDrive*> cache::USBDrivePointersCache;
std::vector<USBDrive*> cache::USBDevices;

WYLAND_END
