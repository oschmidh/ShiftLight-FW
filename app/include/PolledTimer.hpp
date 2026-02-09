#ifndef APP_INCLUDE_POLLEDTIMER_HPP
#define APP_INCLUDE_POLLEDTIMER_HPP

#include <chrono>
#include <cstdint>

template <typename CLOCK_T>
class PolledTimer {
  public:
    template <typename REP_T, typename PERIOD_T>
    PolledTimer(const CLOCK_T& clock, std::chrono::duration<REP_T, PERIOD_T> period) noexcept
     : _clock(clock)
     , _start(_clock.now())
     , _period(std::chrono::duration_cast<typename CLOCK_T::duration>(period))
    { }

    void poll(auto&& action) noexcept
    {
        if (!isElapsed()) {
            return;
        }
        reload();
        action();
    }

  private:
    bool isElapsed() const noexcept
    {
        const auto current = _clock.now();
        return current - _start >= _period;
    }

    void reload() noexcept { _start = _clock.now(); }

    const CLOCK_T& _clock;
    CLOCK_T::time_point _start;
    const CLOCK_T::duration _period;
};

#endif // APP_INCLUDE_POLLEDTIMER_HPP
