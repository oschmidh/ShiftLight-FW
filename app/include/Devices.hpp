#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include <mspm0/SysControl.hpp>
#include <mspm0/IoMux.hpp>
#include <mspm0/Gpio.hpp>
#include <mspm0/I2c.hpp>
#include <mspm0/CaptureTimer.hpp>
#include <mspm0/PeriodicTimer.hpp>

namespace Devices {

extern mspm0::SysControl sysCtl;

extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm1>* pin1;
extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm2>* pin2;
extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm28>* pin28;

extern mspm0::Gpio gpioA;
extern I2c i2c0;
extern mspm0::CaptureTimer<{.intLine = TIMG8_INT_IRQn, .channel = 1, .prescaler = 0xff}> timG8;
extern mspm0::PeriodicTimer<{.intLine = TIMA0_INT_IRQn, .channel = 0, .prescaler = 0xff}> timA0;

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
