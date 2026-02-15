#ifndef LIB_INCLUDE_BUSYWAIT_HPP
#define LIB_INCLUDE_BUSYWAIT_HPP

#include <chrono>

namespace System {

template <typename CLOCK_T, typename REP_T, typename PERIOD_T>
static void busyWait(const CLOCK_T& clock, std::chrono::duration<REP_T, PERIOD_T> duration) noexcept
{
    const auto start = clock.now();
    while (clock.now() < (start + duration))
        ;
}

}    // namespace System

#endif    // LIB_INCLUDE_BUSYWAIT_HPP
