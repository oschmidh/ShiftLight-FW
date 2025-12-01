#ifndef LIB_INCLUDE_CLOCK_HPP
#define LIB_INCLUDE_CLOCK_HPP

#include <chrono>
#include <cstdint>

template <typename TIMER_T>
class TimerSteadyClock {
  public:
    using rep = std::uint64_t;
    using period = std::ratio<TIMER_T::presc, TIMER_T::clkFreq>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<TimerSteadyClock>;
    static const bool is_steady = true;

    static time_point now() noexcept { return time_point(duration(elapsedTicks + TIMER_T::getTicks())); }

    static void init() noexcept { TIMER_T::init(&overflowIsr); }

  private:
    static void overflowIsr() noexcept { elapsedTicks += TIMER_T::period; }

    inline static volatile rep elapsedTicks{};
};

#endif // LIB_INCLUDE_CLOCK_HPP
