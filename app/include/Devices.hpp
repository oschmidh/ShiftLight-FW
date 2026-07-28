#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include <Mspm0c110x.hpp>

#include <mspm0/I2cController.hpp>
#include <mspm0/CaptureTimer.hpp>
#include <mspm0/PeriodicTimer.hpp>

namespace Devices {

extern mspm0::I2cController i2c;
extern mspm0::CaptureTimer<{.intLine = timG8IrqLine, .channel = 1, .prescaler = 0xff}> rpmCaptureTim;
extern mspm0::PeriodicTimer<{.intLine = timA0IrqLine, .channel = 0, .prescaler = 0xff}> sysTim;

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
