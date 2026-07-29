#ifndef LIB_INCLUDE_MSPM0_IOMUX_HPP
#define LIB_INCLUDE_MSPM0_IOMUX_HPP

#include "detail/CommonRegs.hpp"

#include <array>
#include <new>
#include <cstdint>

namespace mspm0::ioMux {

namespace detail {

template <auto>
struct AlternatePinFunctions;

}    // namespace detail

template <auto PIN_V>
class Pin {
  public:
    struct Config {
        typename detail::AlternatePinFunctions<PIN_V>::Type function;
        bool connected = false;
        bool inputEnable = false;
        bool hysteresis = true;
        bool openDrain = false;
    };

    void configure(Config cfg) noexcept
    {
        PINCM = (cfg.openDrain << 25u) | (!cfg.hysteresis << 19u) | (cfg.inputEnable << 18u) | (cfg.connected << 7u) |
                std::to_underlying(cfg.function);
    }

  private:
    volatile std::uint32_t PINCM;
};

enum class Pin1Functions : std::uint32_t {
    Unconnected = 0,
    GpioA_Dio0 = 1,
    I2c0_Sda = 3,
};

enum class Pin2Functions : std::uint32_t {
    Unconnected = 0,
    I2c0_Scl = 3,
};

enum class Pin28Functions : std::uint32_t {
    Unconnected = 0,
    TimG8_Ccp1 = 2,
};

enum class Pins : std::uint32_t {
    PinCm1 = 0,
    PinCm2 = 1,
    PinCm28 = 27,
};

namespace detail {

template <>
struct AlternatePinFunctions<Pins::PinCm1> {
    using Type = Pin1Functions;
};

template <>
struct AlternatePinFunctions<Pins::PinCm2> {
    using Type = Pin2Functions;
};

template <>
struct AlternatePinFunctions<Pins::PinCm28> {
    using Type = Pin28Functions;
};

template <Pins PIN_V>
struct AddressOffset {
    static constexpr std::uintptr_t pinCmOffset = 0x4;
    static constexpr std::uintptr_t value = pinCmOffset + std::to_underlying(PIN_V) * sizeof(Pin<PIN_V>);
};

}    // namespace detail

}    // namespace mspm0::ioMux

#endif    // LIB_INCLUDE_MSPM0_IOMUX_HPP
