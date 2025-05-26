#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.hpp"

#include "../bios/bios_usb_types.h"
#include "../cache.hpp"
#include "cache.hpp"

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

    const USBPacket packet = endpoint_buffers.front();
    endpoint_buffers.erase(endpoint_buffers.begin());
    return packet;
  }

  virtual void Reset() {
    endpoint_buffers.clear();
  }

  // For more complex devices, override these methods
  virtual void SendByte(USBByte byte) {
    USBPacket packet {
      .Bytes = &byte,
      .Len = 1
    };

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

typedef USBPacket (*USBExternalHandleSignPacket)(void);
typedef void (*USBExternalHandleArgPacket)(USBPacket);

typedef bool        (*USBExternalHandleSignBool)();
typedef void        (*USBExternalHandle)();
typedef const char* (*USBExternalHandleSignStr)();
typedef void        (*USBExternalHandleArgByte)(uint8_t);
typedef uint8_t     (*USBExternalHandleSignByte)();
typedef uint32_t    (*USBExternalHandleSignU32)();

class USBDevice;

class IExternalUSBDrive : public USBDrive {
private:
  USBExternalHandleSignBool           Einit = nullptr;
  USBExternalHandle                   Eshutdown = nullptr;
  USBExternalHandleSignStr            Ename = nullptr;
  USBExternalHandleArgPacket          Esend_data = nullptr;
  USBExternalHandleSignPacket         Ereceive_data = nullptr;
  USBExternalHandle                   Ereset = nullptr;
  USBExternalHandleArgByte            Esend_byte = nullptr;
  USBExternalHandleSignByte           Ereceive_byte = nullptr;
  USBExternalHandleSignU32            Eget_device_descriptor = nullptr;

public:
  void Load(const std::string &path) {
    DynamicLibraryHandle handle = DynamicLibraryLoad(path.c_str());

    if (!handle) {
      throw USBException("invalid dynamic handle", memberofstr);
    }

    cache::WylandMMIOModuleHandles.push_back(handle);

    Einit                     = reinterpret_cast<USBExternalHandleSignBool>(DynamicLibraryFunc(handle, "Einit"));
    Eshutdown                 = reinterpret_cast<USBExternalHandle>(DynamicLibraryFunc(handle, "Eshutdown"));
    Ename                     = reinterpret_cast<USBExternalHandleSignStr>(DynamicLibraryFunc(handle, "Ename"));
    Esend_data                = reinterpret_cast<USBExternalHandleArgPacket>(DynamicLibraryFunc(handle, "Esend_data"));
    Ereceive_data             = reinterpret_cast<USBExternalHandleSignPacket>(DynamicLibraryFunc(handle, "Ereceive_data"));
    Ereset                    = reinterpret_cast<USBExternalHandle>(DynamicLibraryFunc(handle, "Ereset"));
    Esend_byte                = reinterpret_cast<USBExternalHandleArgByte>(DynamicLibraryFunc(handle, "Esend_byte"));
    Ereceive_byte             = reinterpret_cast<USBExternalHandleSignByte>(DynamicLibraryFunc(handle, "Ereceive_byte"));
    Eget_device_descriptor    = reinterpret_cast<USBExternalHandleSignU32>(DynamicLibraryFunc(handle, "Eget_device_descriptor"));

    if (!Einit || !Eshutdown || !Ename || !Esend_data || !Ereceive_data ||
        !Ereset || !Esend_byte || !Ereceive_byte || !Eget_device_descriptor) {
      throw USBException("failed to load external USB drive functions", memberofstr);
    }
  }

  wbool init() override {
    return Einit();
  }

  void shutdown() override {
    Eshutdown();
  }

  std::string name() override {
    return Ename();
  }

  void SendData(const USBPacket &) override {
    // TODO
  }

  USBPacket ReceiveData() override {
    return Ereceive_data();
  }

  void Reset() override {
    Ereset();
  }

  void SendByte(USBByte byte) override {
    Esend_byte(byte);
  }

  USBByte ReceiveByte() override {
    wulong data = Ereceive_byte();
    if (data == static_cast<wulong>(-1)) {
      throw USBException("No byte available to receive", memberofstr);
    }

    return (data);
  }

  wuint GetDeviceDescriptor() override {
    return (Eget_device_descriptor());
  }

  USBDrive *Instantiate(const std::string &_Args) override {
    IExternalUSBDrive *IEUSBD = new IExternalUSBDrive();
    cache::USBDrivePointersCache.push_back(IEUSBD);
    IEUSBD->Load(_Args);
    return IEUSBD;
  }
};

WYLAND_END