#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.hpp"

#include "../bios/bios_usb_types.h"

WYLAND_BEGIN

class USBException : public runtime::wyland_runtime_error {
public:
  USBException(const std::string &what, const std::string &from) 
    : runtime::wyland_runtime_error(what.c_str(), "USB Exception", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0) 
  {}
};

// USB Drive base class
class USBDrive : public WylandMMIOModule {
protected:
  std::vector<USBPacket> endpoint_buffers;
  wulong max_packet_size;
  
public:
  USBDrive() : max_packet_size(64) {}
  USBDrive(size_t max_packet_size) : max_packet_size(max_packet_size) {}

  virtual void SendData(const USBPacket& packet) {    
    endpoint_buffers.push_back(packet);
  }

  virtual USBPacket ReceiveData() {
    if (endpoint_buffers.empty()) {
      throw USBException("No data available to receive", memberofstr);
    }

    USBPacket packet = endpoint_buffers.front();
    endpoint_buffers.erase(endpoint_buffers.begin());
    return packet;
  }

  virtual void Reset() {
    endpoint_buffers.clear();
  }

  // For more complex devices, override these methods
  virtual void SendByte(USBByte byte) {
    USBPacket packet {
      .Bytes = new USBByte[1](), 
      .Len = 1
    };

    packet.Bytes[0] = byte;

    SendData(packet);
  }

  virtual USBByte ReceiveByte() {
    USBPacket packet = ReceiveData();
    
    if (packet.Len != 1) {
      throw USBException("Expected single byte, received multi-byte packet", memberofstr);
    }

    return packet.Bytes[0];
  }

  virtual wuint GetDeviceDescriptor() { return 0x0000; }
  virtual std::string name() override { return typeid(this).name(); }
  virtual wbool init() override { return wyland_true; }
  virtual void shutdown() override {}
  virtual USBDrive *Instantiate(const std::string &) { 
    USBDrive *Ptr = new USBDrive();
    cache::USBDrivePointersCache.push_back(Ptr);
    return Ptr;
  }
};

WYLAND_END