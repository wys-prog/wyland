#pragma once
#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.hpp"

#pragma region TODO

/*
  in: (?)/updater.cpp/(?)/.../: add "source" key, that represent Wyland's source DIR.
*/

#pragma endregion

WYLAND_BEGIN

class USBException : public runtime::wyland_runtime_error {
public:
  USBException(const std::string &what, const std::string &from) 
  : runtime::wyland_runtime_error(what.c_str(), "USB Exception", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0) 
  {}
};

// USB exception
class USBException : public runtime::wyland_runtime_error {
public:
  USBException(const std::string &what, const std::string &from) 
    : runtime::wyland_runtime_error(what.c_str(), "USB Exception", from.c_str(), typeid(this).name(), 0, 0, nullptr, nullptr, 0) 
  {}
};

// USB data structures
typedef uint8_t USBByte;
typedef std::vector<USBByte> USBPacket;

// USB Drive base class
class USBDrive : public WylandMMIOModule {
protected:
  std::vector<USBPacket> endpoint_buffers;
  wulong max_packet_size;
  
public:
  USBDrive() : max_packet_size(64) {}
  USBDrive(size_t max_packet_size) : max_packet_size(max_packet_size) {}

  virtual void SendData(const USBPacket& packet) {
    if (packet.size() > max_packet_size) {
      throw USBException("Packet size exceeds maximum packet size", memberofstr);
    }
    
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
    SendData({byte});
  }

  virtual USBByte ReceiveByte() {
    USBPacket packet = ReceiveData();
    if (packet.size() != 1) {
      throw USBException("Expected single byte, received multi-byte packet", memberofstr);
    }
    return packet[0];
  }

  virtual wuint GetDeviceDescriptor() { return 0x0000; }
  virtual std::string name() override { return typeid(this).name(); }
  virtual wbool init() override { return wyland_true; }
  virtual void shutdown() override {}
};

class IExternalUSBDrive : public USBDrive {
private:
  EMMIOFuncSingBool Einit = nullptr;
  EMMIOFunc Eshutdown = nullptr;
  EMMIOFuncSignStr Ename = nullptr;
  EMMIOFuncArgU64 Esend_data = nullptr;
  EMMIOFuncSignU64 Ereceive_data = nullptr;
  EMMIOFunc Ereset = nullptr;
  EMMIOFuncArgByte Esend_byte = nullptr; 
  EMMIOFuncSignByte Ereceive_byte = nullptr;
  // TODO: Send packet !!
  EMMIOFuncSignU32 Eget_device_descriptor = nullptr;

public:
  void Load(const std::string &path) {
    DynamicLibraryHandle handle = DynamicLibraryLoad(path.c_str());

    if (!handle) {
      throw USBException("invalid dynamic handle", memberofstr);
    }

    cache::WylandMMIOModuleHandles.push_back(handle);

    Einit = reinterpret_cast<EMMIOFuncSingBool>(DynamicLibraryFunc(handle, "Einit"));
    Eshutdown = reinterpret_cast<EMMIOFunc>(DynamicLibraryFunc(handle, "Eshutdown"));
    Ename = reinterpret_cast<EMMIOFuncSignStr>(DynamicLibraryFunc(handle, "Ename"));
    Esend_data = reinterpret_cast<EMMIOFuncArgU64>(DynamicLibraryFunc(handle, "Esend_data"));
    Ereceive_data = reinterpret_cast<EMMIOFuncSignU64>(DynamicLibraryFunc(handle, "Ereceive_data"));
    Ereset = reinterpret_cast<EMMIOFunc>(DynamicLibraryFunc(handle, "Ereset"));
    Esend_byte = reinterpret_cast<EMMIOFuncArgByte>(DynamicLibraryFunc(handle, "Esend_byte"));
    Ereceive_byte = reinterpret_cast<EMMIOFuncSignByte>(DynamicLibraryFunc(handle, "Ereceive_byte"));
    Eget_device_descriptor = reinterpret_cast<EMMIOFuncSignU32>(DynamicLibraryFunc(handle, "Eget_device_descriptor"));

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

  void SendData(const USBPacket& packet) override {
    for (USBByte byte : packet) {
      Esend_data(byte);
    }
  }

  USBPacket ReceiveData() override {
    USBPacket packet;
    wulong data = Ereceive_data();
    while (data != (wulong)(-1)) {
      packet.push_back((data));
      data = Ereceive_data();
    }
    return packet;
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
};

WYLAND_END