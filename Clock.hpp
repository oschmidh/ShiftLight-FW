#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <chrono>
#include <cstdint>

template <typename TIMER_T>
class TimerSteadyClock {
  public:
    using rep = std::uint64_t;
    using period = std::ratio<TIMER_T::presc, TIMER_T::clkFreq>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<GenericSteadyClock>;
    static const bool is_steady = true;

    static time_point now() noexcept { return elapsed + duration(TIMER_T::getTicks()); }

    static void init() noexcept { TIMER_T::init(); }

    static constexpr void overflowIsr() { elapsed += duration(TIMER_T::period); }    // TODO should be private

  private:
    inline static time_point elapsed;
};

#endif    // CLOCK_HPP
