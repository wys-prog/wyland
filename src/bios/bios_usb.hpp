#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "../wmmbase.hpp"
#include "../wmmio.hpp"
#include "../wyland-runtime/wylrt.hpp"

#include "bios_usb_types.h"

#include "cache.hpp"

#include "../def/usbdef.hpp"

#pragma region TODO

/*
  in: (?)/updater.cpp/(?)/.../: add "source" key, that represent Wyland's source DIR.
*/

#pragma endregion

WYLAND_BEGIN

typedef USBPacket (*USBExternalHandleSignPacket)(void);
typedef void (*USBExternalHandleArgPacket)(USBPacket);

typedef EMMIOFuncSignBool             USBExternalHandleSignBool;
typedef EMMIOFunc                     USBExternalHandle;
typedef EMMIOFuncSignStr              USBExternalHandleSignStr;
typedef EMMIOFuncArgByte              USBExternalHandleArgByte;
typedef EMMIOFuncSignByte             USBExternalHandleSignByte;
typedef EMMIOFuncSignU32              USBExternalHandleSignU32;

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