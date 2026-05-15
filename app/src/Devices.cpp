#include "Devices.hpp"

static constexpr std::uintptr_t sysCtlAddr = 0x400af000;
mspm0::SysControl Devices::sysCtl(sysCtlAddr);

static constexpr std::uintptr_t ioMuxAddr = 0x40428000;
mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm1> Devices::pin1(ioMuxAddr);
mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm2> Devices::pin2(ioMuxAddr);
mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm28> Devices::pin28(ioMuxAddr);

static constexpr std::uintptr_t gpioAAddr = 0x400a0000;
mspm0::Gpio Devices::gpioA(gpioAAddr);

I2c Devices::i2c0;

static constexpr std::uintptr_t timG8Addr = 0x40090000;
mspm0::CaptureTimer<{.intLine = TIMG8_INT_IRQn, .channel = 1, .prescaler = 0xff}> Devices::timG8(timG8Addr);

static constexpr std::uintptr_t timA0Addr = 0x40860000;
mspm0::PeriodicTimer<{.intLine = TIMA0_INT_IRQn, .channel = 0, .prescaler = 0xff}> Devices::timA0(timA0Addr);
