#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include "Interrupt.hpp"

#include <mspm0/I2c.hpp>
#include <mspm0/CaptureTim.hpp>
#include <mspm0/TimA0Clock.hpp>

namespace Devices {

extern I2c i2c0;
extern CaptureTimG timG8;
extern TimA0Clock timA0;

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
