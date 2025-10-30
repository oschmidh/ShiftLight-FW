#ifndef TIMER_HPP
#define TIMER_HPP

#include "System.hpp"

#include <chrono>
#include <cstdint>

class Timer {
  public:
    template <typename REP_T, typename PERIOD_T>
    Timer(std::chrono::duration<REP_T, PERIOD_T> period) noexcept
     : _start(System::SteadyClock::now())
     , _period(std::chrono::duration_cast<System::SteadyClock::duration>(period))
    { }

    bool isElapsed() const noexcept
    {
        const auto current = System::SteadyClock::now();
        return current - _start >= _period;
    }

    void reload() noexcept { _start = System::SteadyClock::now(); }

    void poll(auto&& action) noexcept    // TODO find proper name
    {
        if (!isElapsed()) {
            return;
        }
        reload();
        action();
    }

  private:
    System::SteadyClock::time_point _start;
    const System::SteadyClock::duration _period;
};

#endif    // TIMER_HPP
