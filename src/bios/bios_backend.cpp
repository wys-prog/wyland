#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../filestream.hpp"
#include "../interfaces/interface.hpp"
#include "../wyland-runtime/wylrt.h"
#include "../wyland-runtime/wfloats.h"
#include "../wyland-runtime/wylrt.hpp"

#include "bios_backend.hpp"
#include "bios.hpp"
#include "bios_disk.hpp"
#include "bios_usb.hpp"

typedef void(*syscall_callable)(wyland_registers*);

WYLAND_BEGIN

namespace bios {
  namespace handles {
    IWylandGraphicsModule *GraphicsModule;
    WylandMMIOModule      *DiskPtr;
    uint8_t               *Memory;
    BiosDiskHandler       *DiskHandler;
    std::vector<USBDrive*> USBDrives;
  }

  class BiosException : public runtime::wyland_runtime_error {
  private:
  public:
    BiosException(const std::string &what, const std::string &from)
      : runtime::wyland_runtime_error(what.c_str(), "BIOS Exception", from.c_str(), typeid(this).name(), 0, 0, NULL, NULL, -1) {}
  };

  void bwrite(wyland_registers *regs) {
    auto bin = to_bin(*regs->r32[0]);
    (handles::GraphicsModule->get_stream()->write((char*)bin, sizeof(wuint)));
    delete bin;
  }

  void bread(wyland_registers *regs) {
    *regs->r32[0] = (wuint)getc(stdin); // WAY !! STDOUT ????? NAAAAAA
  }

  void bgettime(wyland_registers *regs) {
    auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(time_since_epoch).count();
    
    // Convert to big-endian if the system is little-endian
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) {
      nanoseconds = __builtin_bswap64(nanoseconds);
    }
    
    *regs->r64[0] = nanoseconds;
  }

  void bgetdisk(wyland_registers *regs) {
  }
  
  void busb_write_byte(wyland_registers *regs) {
    handles::USBDrives[*(regs->r64[0])]->SendByte(*(regs->r8[0]));
  }

  void busb_read_byte(wyland_registers *regs) {
    *(regs->r8[0]) = handles::USBDrives[*(regs->r64[0])]->ReceiveByte();
  }

  void busb_write_array(wyland_registers *regs) {
    
  }

  void busb_read_array(wyland_registers *regs) {
    
  }
  
  syscall_callable syscall_table[] = {
    &bwrite, &bread, &bgettime, &bgetdisk,
  };

  const constexpr static auto max_syscalls = sizeof(wylma::wyland::bios::syscall_table) / sizeof(wylma::wyland::bios::syscall_table[0]);
}

WYLAND_END

extern "C" {
  void bios_backend_interrupt(wuint id, wyland_registers *regs) {
    if (id >= wylma::wyland::bios::max_syscalls) {
      std::stringstream what, name; // Because stringstream are more simple than strings to use...
      what << "invalid interruption: 0x" << std::setw(4) << std::setfill('0') << std::hex << id;
      name << "BIOS/" << __func__;
      throw wylma::wyland::bios::BiosException(what.str(), name.str());
    } else {
      (*wylma::wyland::bios::syscall_table)(regs);
    }
  }

  void bios_backend_init(const std::vector<wylma::wyland::WylandMMIOModule*> &modules, 
                         wylma::wyland::IWylandGraphicsModule *gm, uint8_t *mmptr, 
                         wylma::wyland::WylandMMIOModule *dsptr, const std::vector<wylma::wyland::USBDrive*> &usbdrives) {
    std::cout << "[b]: BIOS version: " << (bios_backend_version()) << std::endl;
    
    for (const auto&module:modules) {
      std::cout << "[b]: initializing module " << module->name() << ":\t" << std::flush;
      if (!module->init()) {
        std::cout << "[FAILLURE]" << std::endl;
        throw wylma::wyland::MMIOModuleException("Unable to initialize MMIO Module: " + module->name(), "BIOS/Backend/" + std::string(__func__));
      } else std::cout << "[SUCCESS]" << std::endl;
    }

    for (const auto&drive:usbdrives) {
      std::cout << "[b]: initializing module " << drive->name() << ":\t" << std::flush;
      if (!drive->init()) {
        std::cout << "[FAILLURE]" << std::endl;
        throw wylma::wyland::USBException("Unable to initialize MMIO Module: " + drive->name(), "BIOS/Backend/" + std::string(__func__));
      } else std::cout << "[SUCCESS]" << std::endl;
    }

    wylma::wyland::bios::handles::GraphicsModule = gm;
    wylma::wyland::bios::handles::Memory = mmptr;
    wylma::wyland::bios::handles::DiskPtr = dsptr;
    wylma::wyland::bios::handles::DiskHandler = new wylma::wyland::bios::BiosDiskHandler(dsptr);
  }

  long double bios_backend_version() {
    return (1.8);
  }
}