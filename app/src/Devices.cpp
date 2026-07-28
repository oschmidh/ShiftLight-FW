#include "Devices.hpp"

#include <Mspm0c110x.hpp>

mspm0::I2cController Devices::i2c(mspm0::peripherals::i2c0);
mspm0::CaptureTimer<{.intLine = timG8IrqLine, .channel = 1, .prescaler = 0xff}> Devices::rpmCaptureTim(
    mspm0::peripherals::timG8);
mspm0::PeriodicTimer<{.intLine = timA0IrqLine, .channel = 0, .prescaler = 0xff}> Devices::sysTim(
    mspm0::peripherals::timA0);
