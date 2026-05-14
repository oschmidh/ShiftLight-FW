#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include <mspm0/I2c.hpp>
#include <mspm0/CaptureTimer.hpp>
#include <mspm0/PeriodicTimer.hpp>

namespace Devices {

extern I2c i2c0;
extern mspm0::CaptureTimer<{.intLine = TIMG8_INT_IRQn, .channel = 1, .prescaler = 0xff}> timG8;
extern mspm0::PeriodicTimer<{.intLine = TIMA0_INT_IRQn, .channel = 0, .prescaler = 0xff}> timA0;

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
