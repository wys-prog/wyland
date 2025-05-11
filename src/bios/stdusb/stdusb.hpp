#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

#include "../../wmmbase.hpp"
#include "../../wyland-runtime/wylrt.hpp"
#include "../bios_usb.hpp"
#include "../bios_usb_types.h"

WYLAND_BEGIN

class ClassicUSBKey : public USBDrive {
private:
  std::string storage_file;
  size_t capacity;
  size_t used_space;

  void UpdateUsedSpace() {
    std::ifstream file(storage_file, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      throw USBException("Unable to open storage file", memberofstr);
    }
    used_space = file.tellg();
    file.close();
  }

public:
  ClassicUSBKey(const std::string &file_path) : storage_file(file_path), capacity(4_GB), used_space(0) {
    init();
  }

  void init(const std::string &path) {
    storage_file = path;
    init();
  }

  ClassicUSBKey() = default;

  size_t GetFreeSpace() const {
    return capacity - used_space;
  }

  void Write(const std::vector<uint8_t>& data) {
    if (data.size() > GetFreeSpace()) {
      throw USBException("Not enough space on USB key", memberofstr);
    }
    std::ofstream file(storage_file, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
      throw USBException("Unable to open storage file for writing", memberofstr);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    used_space += data.size();
    file.close();
  }

  std::vector<uint8_t> Read(size_t length) {
    std::ifstream file(storage_file, std::ios::binary);
    if (!file.is_open()) {
      throw USBException("Unable to open storage file for reading", memberofstr);
    }
    std::vector<uint8_t> buffer(length);
    file.read(reinterpret_cast<char*>(buffer.data()), length);
    buffer.resize(file.gcount());
    file.close();
    return buffer;
  }

  void Reset() override {
    std::ofstream file(storage_file, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
      throw USBException("Unable to reset storage file", memberofstr);
    }
    file.close();
    used_space = 0;
  }

  std::string name() override {
    return typeid(this).name();
  }

  wbool init() override {
    std::ifstream file(storage_file, std::ios::binary);
    if (!file.is_open()) {
      // Create the file if it doesn't exist
      std::ofstream new_file(storage_file, std::ios::binary);
      if (!new_file.is_open()) {
        throw USBException("Failed to create storage file", memberofstr);
      }
      new_file.close();
    } else {
      UpdateUsedSpace();
    }

    return wyland_true;
  }

  wuint GetDeviceDescriptor() override {
    return 0x0001;
  }

  USBDrive *Instantiate(const std::string &_Args) override {
    ClassicUSBKey *Key = new ClassicUSBKey();
    cache::USBDrivePointersCache.push_back(Key);
    Key->init(_Args);
    return Key;
  }
};

std::unordered_map<std::string, USBDrive> stdusb_devices = {
  {"_wClassicUSBKey", ClassicUSBKey()}, 
  {"_wusbkey", ClassicUSBKey()},
  {"_wdefault", USBDrive()},
};

WYLAND_END
