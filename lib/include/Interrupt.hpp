#ifndef LIB_INCLUDE_INTERRUPT_HPP
#define LIB_INCLUDE_INTERRUPT_HPP

#include <array>
#include <tuple>
#include <utility>
#include <type_traits>

namespace System {

namespace detail {

template <unsigned int ISR_NUM_V, typename... Ts>
struct FindIsrDevs;

template <unsigned int ISR_NUM_V, typename FIRST_T, typename... REST_Ts>
struct FindIsrDevs<ISR_NUM_V, FIRST_T, REST_Ts...> {
    using Type = typename FindIsrDevs<ISR_NUM_V, REST_Ts...>::Type;
};

template <unsigned int ISR_NUM_V, typename FIRST_T, typename... REST_Ts>
    requires(FIRST_T::intLine == ISR_NUM_V)
struct FindIsrDevs<ISR_NUM_V, FIRST_T, REST_Ts...> {
    static_assert(requires(FIRST_T dev) { dev.isr(); }, "Device specifies Interrupt line, but no ISR function");
    using Type = decltype(std::tuple_cat(std::declval<std::tuple<FIRST_T>>(),
                                         std::declval<typename FindIsrDevs<ISR_NUM_V, REST_Ts...>::Type>()));
};

template <unsigned int ISR_NUM_V>
struct FindIsrDevs<ISR_NUM_V> {
    using Type = std::tuple<>;
};

}    // namespace detail

template <unsigned int NUM_LINES_V, auto&... DEVICE_Vs>
struct InterruptHandler {
  public:
    static constexpr std::array<void (*)(), NUM_LINES_V> createIsrTable() noexcept
    {
        return []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
            return std::array<void (*)(), NUM_LINES_V>{getIsr<IDX_Vs>()...};
        }(std::make_index_sequence<NUM_LINES_V>());
    }

  private:
    template <unsigned int LINE_V>
    static constexpr auto getIsr() noexcept -> void (*)()
    {
        using Devices = detail::FindIsrDevs<LINE_V, typename std::remove_cvref<decltype(DEVICE_Vs)>::type...>::Type;
        return &isr<Devices>;
    }

    template <typename DEV_TUPLE_T>
        requires(std::tuple_size_v<DEV_TUPLE_T> > 0)
    static void isr() noexcept
    {
        return []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
            (std::get<std::tuple_element_t<IDX_Vs, DEV_TUPLE_T>&>(std::tie(DEVICE_Vs...)).isr(), ...);
        }(std::make_index_sequence<std::tuple_size_v<DEV_TUPLE_T>>());
    }

    // empty default handler:
    template <typename DEV_TUPLE_T>
        requires(!std::tuple_size_v<DEV_TUPLE_T>)
    static void isr() noexcept
    { }
};

}    // namespace System

#endif    // LIB_INCLUDE_INTERRUPT_HPP
