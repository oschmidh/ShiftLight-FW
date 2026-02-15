#ifndef LIB_INCLUDE_TIMERCLOCK_HPP
#define LIB_INCLUDE_TIMERCLOCK_HPP

#include <chrono>
#include <cstdint>

template <auto& TIMER_V>
class TimerSteadyClock {
    using TimerType = std::remove_cvref_t<decltype(TIMER_V)>;

  public:
    using rep = std::uint64_t;
    using period = std::ratio<TimerType::presc, TimerType::clkFreq>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<TimerSteadyClock>;
    static const bool is_steady = true;

    static time_point now() noexcept { return time_point(duration(elapsedTicks + TIMER_V.getTicks())); }

    static void init() noexcept { TIMER_V.setCallback(&overflowIsr); }

  private:
    static void overflowIsr() noexcept { elapsedTicks += TIMER_V.period; }

    inline static volatile rep elapsedTicks{};
};

#endif    // LIB_INCLUDE_TIMERCLOCK_HPP
