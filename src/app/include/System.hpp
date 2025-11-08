#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "TimA0Clock.hpp"
#include "Clock.hpp"

#include <chrono>

namespace System {

using SteadyClock = TimerSteadyClock<TimA0Clock>;

template <typename REP_T, typename PERIOD_T>
static void busyWait(std::chrono::duration<REP_T, PERIOD_T> duration) noexcept
{
    const auto start = SteadyClock::now();
    while (SteadyClock::now() < (start + duration))
        ;
}

}    // namespace System

#endif    // SYSTEM_HPP
