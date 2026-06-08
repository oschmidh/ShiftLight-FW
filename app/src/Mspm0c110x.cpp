#include "Mspm0c110x.hpp"

namespace {

namespace PeriphAddr {

static constexpr std::uintptr_t timG8 = 0x40090000;
static constexpr std::uintptr_t gpio0 = 0x400a0000;
static constexpr std::uintptr_t sysCtl = 0x400af000;
static constexpr std::uintptr_t i2c0 = 0x400f0000;
static constexpr std::uintptr_t ioMux = 0x40428000;
static constexpr std::uintptr_t timA0 = 0x40860000;

};    // namespace PeriphAddr

template <mspm0::IoMux::Pins PIN_V>
mspm0::IoMux::Pin<PIN_V>* createPin(std::uintptr_t ioMuxAddr) noexcept
{
    using PinType = mspm0::IoMux::Pin<PIN_V>;
    return new (reinterpret_cast<PinType*>(ioMuxAddr + mspm0::IoMux::detail::AddressOffset<PIN_V>::value)) PinType;
}

}    // namespace

namespace mspm0::peripherals {

mspm0::SysControl sysCtl(PeriphAddr::sysCtl);
mspm0::Gpio gpio0(PeriphAddr::gpio0);
mspm0::I2c i2c0(PeriphAddr::i2c0);
mspm0::Timer timG8(PeriphAddr::timG8);
mspm0::Timer timA0(PeriphAddr::timA0);

mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm1>* pin1 = createPin<mspm0::IoMux::Pins::PinCm1>(PeriphAddr::ioMux);
mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm2>* pin2 = createPin<mspm0::IoMux::Pins::PinCm2>(PeriphAddr::ioMux);
mspm0::IoMux::Pin<mspm0::IoMux::Pins::PinCm28>* pin28 = createPin<mspm0::IoMux::Pins::PinCm28>(PeriphAddr::ioMux);

}    // namespace mspm0::peripherals
