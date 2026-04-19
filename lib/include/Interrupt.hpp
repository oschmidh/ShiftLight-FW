#ifndef LIB_INCLUDE_INTERRUPT_HPP
#define LIB_INCLUDE_INTERRUPT_HPP

#include "Typelist.hpp"

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
    using Type = Append<FIRST_T, typename FindIsrDevs<ISR_NUM_V, REST_Ts...>::Type>::type;
};

template <unsigned int ISR_NUM_V>
struct FindIsrDevs<ISR_NUM_V> {
    using Type = Typelist<>;
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

    template <typename DEV_LIST_T>
        requires(!IsEmpty<DEV_LIST_T>::value)
    static void isr() noexcept
    {
        return []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
            (std::get<typename At<IDX_Vs, DEV_LIST_T>::type&>(std::tie(DEVICE_Vs...)).isr(), ...);
        }(std::make_index_sequence<Size<DEV_LIST_T>::value>());
    }

    // empty default handler:
    template <typename DEV_LIST_T>
        requires(IsEmpty<DEV_LIST_T>::value)
    static void isr() noexcept
    { }
};

}    // namespace System

#endif    // LIB_INCLUDE_INTERRUPT_HPP
