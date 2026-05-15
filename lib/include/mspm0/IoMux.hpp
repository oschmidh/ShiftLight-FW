#ifndef LIB_INCLUDE_MSPM0_IOMUX_HPP
#define LIB_INCLUDE_MSPM0_IOMUX_HPP

#include "RegSet.hpp"

#include <array>
#include <new>
#include <cstdint>

namespace mspm0 {

namespace IoMux {

// template <typename ALT_FN_ENUM_T>
// struct Pin {
//     using PinFunctions = ALT_FN_ENUM_T;
// };
namespace detail {

template <auto>
struct AlternatePinFunctions;

// template <auto>
// struct PinCmOffset;

}    // namespace detail

template <auto PIN_V>
class Pin {
  public:
    struct Config {
        typename detail::AlternatePinFunctions<PIN_V>::Type function;
        bool connected = false;
        bool inputEnable = false;
        bool openDrain = false;
    };

    void configure(Config cfg) noexcept
    {
        PINCM = (cfg.openDrain << 25u) | (cfg.inputEnable << 18u) | (cfg.connected << 7u) |
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

// static constexpr Pin<Cm1PinFunctions> cm1{0};
// static constexpr Pin<Cm2PinFunctions> cm2{1};

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

// template <>
// struct detail::PinCmOffset<Pins::PinCm1> {
//     static constexpr std::uintptr_t value = 0;
// };

// template <>
// struct detail::PinCmOffset<Pins::PinCm2> {
//     static constexpr std::uintptr_t value = 1;
// };

// template <>
// struct detail::PinCmOffset<Pins::PinCm28> {
//     static constexpr std::uintptr_t value = 27;
// };

}    // namespace IoMux

// class IoMux {
//   public:
//     template <auto PIN_V>
//     class Pin {
//       public:
//         struct Config {
//             typename AlternatePinFunctions<PIN_V>::Type function;
//             bool openDrain = false;
//         };

//         constexpr Pin(uintptr_t addr) noexcept
//          : PINCM(new (reinterpret_cast<std::uint32_t*>(addr + pinCmOffset + PinCmOffset<PIN_V>::value))
//          std::uint32_t)
//         { }

//         void configure(Config cfg) const noexcept { *PINCM = (cfg.openDrain << 25u) | cfg.function; }

//       private:
//         static constexpr uintptr_t pinCmOffset = 0x4;
//         volatile std::uint32_t* PINCM;
//     };

//     // template <std::size_t N>
//     // static constexpr std::array<Pin, N> buildPinArray(std::uintptr_t baseAddr) noexcept
//     // {
//     //     return [&baseAddr]<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
//     //         return std::array{Pin{baseAddr, IDX_Vs}...};
//     //     }(std::make_index_sequence<N>{});
//     // }

//     IoMux(std::uintptr_t addr) noexcept
//      //  : _pins{buildPinArray<18>(addr)}
//      : _pins{{{addr, 0},
//               {addr, 1},
//               {addr, 2},
//               {addr, 4},
//               {addr, 6},
//               {addr, 11},
//               {addr, 16},
//               {addr, 17},
//               {addr, 18},
//               {addr, 19},
//               {addr, 20},
//               {addr, 22},
//               {addr, 23},
//               {addr, 24},
//               {addr, 25},
//               {addr, 26},
//               {addr, 27},
//               {addr, 28}}}
//     { }

//     Pin& getPin(unsigned int pinNr) noexcept { return _pins[pinNr]; }
//     const Pin& getPin(unsigned int pinNr) const noexcept { return _pins[pinNr]; }

//   private:
//     // enum PinCm : std::uint32_t {

//     // }

//     // struct Registers {
//     //     std::uint32_t spare;
//     //     std::uint32_t PINCM[251];    // TODO why 251?
//     // };

//     static constexpr uintptr_t pinCmOffset = 0x4;

//     using PinTuple =

//         std::array<Pin, 18> _pins;    // TODO unnessecary RAM waste
//     //  volatile Registers* const _regs;
// };

// do before init of other peripheral using iomux:
// set PF, PC and INENA in PINCMx

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_IOMUX_HPP
