#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include <mspm0/I2c.hpp>
#include <mspm0/CaptureTim.hpp>
#include <mspm0/TimA0Clock.hpp>

namespace Devices {

extern I2c i2c0;
extern mspm0::CaptureTim<{.intLine = TIMG8_INT_IRQn, .channel = 1, .prescaler = 0xff}> timG8;
extern PeriodicTimer timA0;

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
