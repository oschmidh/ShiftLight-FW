#ifndef APP_INCLUDE_DEVICES_HPP
#define APP_INCLUDE_DEVICES_HPP

#include "Interrupt.hpp"

#include <drivers/Tlc59208f.hpp>
#include <mspm0/I2c.hpp>
#include <mspm0/CaptureTim.hpp>
#include <mspm0/TimA0Clock.hpp>

namespace Devices {

// extern I2c i2c0;
// extern CaptureTimG timG8;
// extern Tlc59208f ledDriver;

// static constexpr DeviceTree dt {I2c0, timG8};

template <std::size_t N>
struct ConstString {
    // constexpr ConstString(const char* str) noexcept { std::copy_n(str, std::strlen(str), arr.begin()); }
    // constexpr ConstString(std::string_view str) noexcept { std::copy(str.begin(), str.end(), arr.begin()); }
    constexpr ConstString(const char (&str)[N]) noexcept { std::copy_n(str, N, arr.begin()); }

    std::array<char, N> arr;
};

// ConstString(const char *) -> ConstString<std::string_view>;

// template <std::size_t SIZE_V>
// struct ConstString->ConstString<SIZE_V>;

// struct Properties {
//     const char* name;
// };

template <ConstString>
struct Label { };

// template <std::size_t N>
// struct Label<ConstString<N>> { };

template <typename DRIVER_T, typename...>
struct Node { };

// template <template<typename...>typename DRIVER_T, typename...>
// struct Node { };

// template <typename DRIVER_T, std::size_t N>
// struct Node<DRIVER_T, Label<ConstString<N>>> { };

// template <typename DRIVER_T>
// struct Node<DRIVER_T, Label> { };

// template <typename... CHILD_NODE_Ts>
// struct Children { };

template <template <typename...> typename DRIVER_T, typename...>
struct Child { };

template <std::uint8_t>
struct Address { };

template <typename... DEV_Ts>
class DeviceTree {
  public:
    constexpr DeviceTree() noexcept
     : _devices()
    { }

    constexpr void init() noexcept
    {
        std::apply([](auto& dev) { dev.init(); }, _devices);    // TODO order?
    }

    template <ConstString LABEL_V>
    constexpr auto& get() noexcept
    {
        return std::get<>(_devices);    // TODO implement
    }

    template <ConstString LABEL_V>
    constexpr const auto& get() const noexcept
    {
        return std::get<>(_devices);    // TODO implement
    }

  private:
    std::tuple<DEV_Ts...> _devices;
};

template <typename... NODE_Ts>
static constexpr std::tuple<NODE_Ts...> createDevicetree() noexcept
{
    return std::tuple<NODE_Ts...>{};
}

// static constexpr auto dt = createDevicetree<Node<I2c, {.name = "i2c0"}>, Node<CaptureTim, {.name = "timG8"}>,
//                                             Node<Tlc59208f, {.name = "ledDriver"}>>();

template <typename T>
struct Test {
    using Type = T::afkjhnioud;
};

template <typename T>
struct IsChild : std::false_type { };

template <template <typename...> typename DRIVER_T, typename... ARG_Vs>
struct IsChild<Child<DRIVER_T, ARG_Vs...>> : std::true_type { };

// namespace Concepts {

// template <typename T>
// concept Child = IsChild<T>::value;

// }

template <template <typename> typename PRED_T, typename TUPLE_T>
struct Filter;

template <template <typename> typename PRED_T, typename FIRST_T, typename... REST_Ts>
struct Filter<PRED_T, std::tuple<FIRST_T, REST_Ts...>> {
    using Type = typename Filter<PRED_T, std::tuple<REST_Ts...>>::Type;
};

template <template <typename> typename PRED_T, typename FIRST_T, typename... REST_Ts>
    requires PRED_T<FIRST_T>::value
struct Filter<PRED_T, std::tuple<FIRST_T, REST_Ts...>> {
    using Type = decltype(std::tuple_cat(std::declval<std::tuple<FIRST_T>>(),
                                         std::declval<typename Filter<PRED_T, std::tuple<REST_Ts...>>::Type>()));
};

template <template <typename> typename PRED_T>
struct Filter<PRED_T, std::tuple<>> {
    using Type = std::tuple<>;
};

template <typename PARENT_T>
struct CollectChildren {
    using Type = std::tuple<>;
};

// template <typename DRIVER_T, typename... ARG1_Ts, Concepts::Child... CHILD_Ts, typename... ARG2_Ts>
// struct CollectChildren<Node<DRIVER_T, ARG1_Ts..., CHILD_Ts..., ARG2_Ts...>> {
//     using Type = std::tuple<CHILD_Ts..., typename CollectChildren<CHILD_Ts>::Type...>;
// };

template <typename DRIVER_T, typename... ARG_Ts>
struct CollectChildren<Node<DRIVER_T, ARG_Ts...>> {
    using Type = typename Filter<IsChild, std::tuple<ARG_Ts...>>::Type;
};

// template <typename T, template<typename>typename PRED_T>
// struct Filter;

// template <typename... Ts, template<typename>typename PRED_T>
// struct Filter<std::tuple<Ts...>, PRED_T>{
//     using Type = std::tuple<>
// }

// WORKING:
// template <typename DRIVER_T, typename LABEL_T, Concepts::Child... CHILD_Ts>
// struct CollectChildren<Node<DRIVER_T, LABEL_T, CHILD_Ts...>> {
//     using Type = std::tuple<CHILD_Ts...>;
// };

// NOT WORKING:
// template <typename DRIVER_T, typename FIRST_T, typename... REST_Ts>
// struct CollectChildren<Node<DRIVER_T, FIRST_T, REST_Ts...>> {
//     using Type = CollectChildren<Node<DRIVER_T, REST_Ts...>>::Type;
// };

// template <typename DRIVER_T, typename... ARG1_Ts, Concepts::Child... CHILD_Ts>
// struct CollectChildren<Node<DRIVER_T, ARG1_Ts..., CHILD_Ts...>> {
//     using Type = std::tuple<CHILD_Ts...>;
// };

// template <typename DRIVER_T, Concepts::Child... CHILD_Ts, typename... REST_Ts>
// struct CollectChildren<Node<DRIVER_T, CHILD_Ts..., REST_Ts...>> {
//     using Type = std::tuple<CHILD_Ts..., typename CollectChildren<CHILD_Ts>::Type...>;
// };

static_assert(IsChild<Child<Tlc59208f, Label<"ledDriver2">, Address<0x21>>>::value);    // TODO test

using Children = typename CollectChildren<Node<I2c, Label<"i2c0">, Child<Tlc59208f, Label<"ledDriver">, Address<0x20>>,
                                               Child<Tlc59208f, Label<"ledDriver2">, Address<0x21>>>>::Type;
using T = Test<Children>::Type;

// clang-format off
static constinit const auto dt = createDevicetree<
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
        Label<"i2c0">,
        Child<
            Tlc59208f,
            Label<"ledDriver">,
            Address<0x20>
        >
    >,

    Node<
        CaptureTimG,
        Label<"timG8">
    >
>();

// clang-format on

}    // namespace Devices

#endif    // APP_INCLUDE_DEVICES_HPP
