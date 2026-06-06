#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include <mspm0/SysControl.hpp>
#include <mspm0/IoMux.hpp>
#include <mspm0/Gpio.hpp>
#include <mspm0/I2cController.hpp>
#include <mspm0/CaptureTimer.hpp>
#include <mspm0/PeriodicTimer.hpp>

static constexpr unsigned int timG8IrqLine = 2;
static constexpr unsigned int timA0IrqLine = 18;

namespace Devices {

extern mspm0::SysControl sysCtl;

extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm1>* pin1;
extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm2>* pin2;
extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm28>* pin28;

extern mspm0::Gpio gpioA;
extern mspm0::I2cController i2c0;
extern mspm0::CaptureTimer<{.intLine = timG8IrqLine, .channel = 1, .prescaler = 0xff}> timG8;
extern mspm0::PeriodicTimer<{.intLine = timA0IrqLine, .channel = 0, .prescaler = 0xff}> timA0;

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
