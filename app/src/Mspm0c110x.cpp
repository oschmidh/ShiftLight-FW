#include "Mspm0c110x.hpp"

#include <new>

namespace {

namespace PeriphAddr {

static constexpr std::uintptr_t nvic = 0xe000e100;

static constexpr std::uintptr_t timG8 = 0x40090000;
static constexpr std::uintptr_t gpio0 = 0x400a0000;
static constexpr std::uintptr_t sysCtl = 0x400af000;
static constexpr std::uintptr_t i2c0 = 0x400f0000;
static constexpr std::uintptr_t ioMux = 0x40428000;
static constexpr std::uintptr_t timA0 = 0x40860000;

};    // namespace PeriphAddr

template <mspm0::ioMux::Pins PIN_V>
mspm0::ioMux::Pin<PIN_V>* createPin(std::uintptr_t ioMuxAddr) noexcept
{
    using PinType = mspm0::ioMux::Pin<PIN_V>;
    return new (reinterpret_cast<PinType*>(ioMuxAddr + mspm0::ioMux::detail::AddressOffset<PIN_V>::value)) PinType;
}

}    // namespace

namespace mspm0::peripherals {

cortex_m0plus::Nvic nvic(PeriphAddr::nvic);

mspm0::SysControl sysCtl(PeriphAddr::sysCtl);
mspm0::Gpio gpio0(PeriphAddr::gpio0, nvic);
mspm0::I2c i2c0(PeriphAddr::i2c0, nvic);
mspm0::Timer timG8(PeriphAddr::timG8, nvic);
mspm0::Timer timA0(PeriphAddr::timA0, nvic);

mspm0::ioMux::Pin<mspm0::ioMux::Pins::PinCm1>& pin1 = *createPin<mspm0::ioMux::Pins::PinCm1>(PeriphAddr::ioMux);
mspm0::ioMux::Pin<mspm0::ioMux::Pins::PinCm2>& pin2 = *createPin<mspm0::ioMux::Pins::PinCm2>(PeriphAddr::ioMux);
mspm0::ioMux::Pin<mspm0::ioMux::Pins::PinCm28>& pin28 = *createPin<mspm0::ioMux::Pins::PinCm28>(PeriphAddr::ioMux);

}    // namespace mspm0::peripherals
