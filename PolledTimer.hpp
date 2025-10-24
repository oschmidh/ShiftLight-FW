#ifndef POLLEDTIMER_HPP
#define POLLEDTIMER_HPP

#include "System.hpp"

#include <chrono>
#include <cstdint>

class PolledTimer {
  public:
    template <typename REP_T, typename PERIOD_T>
    PolledTimer(std::chrono::duration<REP_T, PERIOD_T> period) noexcept
     : _start(System::SteadyClock::now())
     , _period(std::chrono::duration_cast<System::SteadyClock::duration>(period))
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
        const auto current = System::SteadyClock::now();
        return current - _start >= _period;
    }

    void reload() noexcept { _start = System::SteadyClock::now(); }

    System::SteadyClock::time_point _start;
    const System::SteadyClock::duration _period;
};

#endif    // POLLEDTIMER_HPP
