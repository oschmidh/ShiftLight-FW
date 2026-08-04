#ifndef LIB_INCLUDE_CLOCK_HPP
#define LIB_INCLUDE_CLOCK_HPP

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

    static void init() noexcept { TIMER_V.init(&overflowIsr); }

  private:
    static void overflowIsr() noexcept { elapsedTicks += TIMER_V.period; }

    inline static volatile rep elapsedTicks{};
};

template <typename CLOCK_T, typename REP_T, typename PERIOD_T>
static void busyWait(const CLOCK_T& clock, std::chrono::duration<REP_T, PERIOD_T> duration) noexcept
{
    const auto start = clock.now();
    while (clock.now() < (start + duration))
        ;
}

#endif    // LIB_INCLUDE_CLOCK_HPP
