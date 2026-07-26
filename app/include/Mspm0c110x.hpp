#ifndef APP_INCLUDE_MSPM0C110X_HPP
#define APP_INCLUDE_MSPM0C110X_HPP

#include <cortex_m0plus/Nvic.hpp>
#include <mspm0/SysControl.hpp>
#include <mspm0/IoMux.hpp>
#include <mspm0/Gpio.hpp>
#include <mspm0/I2c.hpp>
#include <mspm0/Timer.hpp>

static constexpr unsigned int timG8IrqLine = 2;
static constexpr unsigned int timA0IrqLine = 18;

namespace mspm0::peripherals {

extern cortex_m0plus::Nvic nvic;

extern mspm0::SysControl sysCtl;
extern mspm0::Gpio gpio0;
extern mspm0::I2c i2c0;
extern mspm0::Timer timG8;
extern mspm0::Timer timA0;

extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm1>& pin1;
extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm2>& pin2;
extern mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm28>& pin28;

}    // namespace mspm0::peripherals

#endif    // APP_INCLUDE_MSPM0C110X_HPP
