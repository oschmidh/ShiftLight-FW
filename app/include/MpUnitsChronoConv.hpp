#ifndef APP_INCLUDE_MPUNITSCHRONOCONV_HPP
#define APP_INCLUDE_MPUNITSCHRONOCONV_HPP

#include <mp-units/systems/si.h>
#include <chrono>

// copied from mp-units/systems/si/chrono.h, because that header cannot be used in freestanding mode
namespace mp_units {

namespace detail {

template <typename Period>
[[nodiscard]] consteval auto time_unit_from_chrono_period()
{
    using namespace si;

    if constexpr (std::is_same_v<Period, std::chrono::nanoseconds::period>)
        return nano<second>;
    else if constexpr (std::is_same_v<Period, std::chrono::microseconds::period>)
        return micro<second>;
    else if constexpr (std::is_same_v<Period, std::chrono::milliseconds::period>)
        return milli<second>;
    else if constexpr (std::is_same_v<Period, std::chrono::seconds::period>)
        return second;
    else if constexpr (std::is_same_v<Period, std::chrono::minutes::period>)
        return minute;
    else if constexpr (std::is_same_v<Period, std::chrono::hours::period>)
        return hour;
    else if constexpr (std::is_same_v<Period, std::chrono::days::period>)
        return day;
    else if constexpr (std::is_same_v<Period, std::chrono::weeks::period>)
        return mag<7> * day;
    else
        return mag_ratio<Period::num, Period::den> * second;
}

}    // namespace detail

template <typename Rep, typename Period>
struct quantity_like_traits<std::chrono::duration<Rep, Period>> {
    static constexpr auto reference = detail::time_unit_from_chrono_period<Period>();
    static constexpr bool explicit_import = false;
    static constexpr bool explicit_export = false;
    using rep = Rep;
    using T = std::chrono::duration<Rep, Period>;

    [[nodiscard]] static constexpr rep to_numerical_value(const T& q) noexcept(
        std::is_nothrow_copy_constructible_v<rep>)
    {
        return q.count();
    }

    [[nodiscard]] static constexpr T from_numerical_value(const rep& val) noexcept(
        std::is_nothrow_copy_constructible_v<rep>)
    {
        return T(val);
    }
};

}    // namespace mp_units

#endif    // APP_INCLUDE_MPUNITSCHRONOCONV_HPP
