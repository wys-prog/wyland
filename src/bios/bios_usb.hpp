#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "../wmmbase.hpp"

#include "../def/usbdef.hpp"
#include "../def/usbEXdef.hpp"

#pragma region TODO

/*
  in: (?)/updates.cpp/(?)/.../: add "source" key, that represent Wyland's source DIR.
*/

#pragma endregion

WYLAND_BEGIN

class USBDrive;
class USBException;
class IExternalUSBDrive;

WYLAND_END