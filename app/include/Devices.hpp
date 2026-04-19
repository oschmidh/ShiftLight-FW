#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

// #include "Interrupt.hpp"

#include <drivers/Tlc59208f.hpp>
#include <mspm0/I2c.hpp>
#include <mspm0/CaptureTim.hpp>
#include <mspm0/TimA0Clock.hpp>

#include "Devicetree.hpp"

// TODO move to separate file?

// enum class DtParameters{
//     Address,
// };

template <typename I2C_T>
struct CtorArgs<Tlc59208f<I2C_T>> {
    // template <std::uint8_t ADDR_V>
    // using type = Typelist<Address<ADDR_V>>;
    using type = Typelist<Address<std::uint8_t>>;
};

// struct Binding {
//     std::uint8_t address;
// };

// template <typename I2C_T>
// struct CreateBinding<Tlc59208f<I2C_T>> {

//     template <typename T>
//     static constexpr Binding value = {.address = Address<>};
//     using type = Typelist<Address>;
// };

namespace Devices {

// clang-format off
using Dt = Devicetree<
    // Node<
    //     I2c,
    //     Label<"i2c0">,
    //     Children<
    //         Node<
    //             Tlc59208f,
    //             Label<"ledDriver">,
    //             Address<0x20>
    //         >
    //     >
    // >,

    Node<
        I2c,
        Label<"i2c0">{},

        Child<
            Tlc59208f,
            Label<"ledDriver">{},
            // {.address = 0x20}
            Address<std::uint8_t>{0x20}
        >{}
    >,

    Node<
        TimA0Clock,
        Label<"timA0">{},
        Interrupt<TIMA0_INT_IRQn>{}
    >,

    Node<
        CaptureTimG,
        Label<"timG8">{},
        Interrupt<TIMG8_INT_IRQn>{}
    >
>;
// clang-format on

}    // namespace Devices

// static constinit System_<Devices::Dt> devices{};
static DeviceContainer<Devices::Dt> devices{};

#endif    // APP_INCLUDE_DEVICES_HPP
