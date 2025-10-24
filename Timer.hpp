#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <cstdint>

template <typename SYS_TIM_T>
class Timer {
  public:
    template <typename REP_T, typename PERIOD_T>
    Timer(const SYS_TIM_T& sysTim, std::chrono::duration<REP_T, PERIOD_T> period) noexcept
     : Timer(sysTim, SYS_TIM_T::toTicks(period))
    { }

    Timer(const SYS_TIM_T& sysTim, SYS_TIM_T::TickType period) noexcept
     : _start(now())
     , _period(period)
     , _sysTim(sysTim)
    { }

    bool isElapsed() const noexcept
    {
        const auto current = now();
        if (_start > current) {    // handle overflow
            return current + _sysTim.getPeriod() - _start >= _period;
        }

        return current - _start >= _period;
    }

    void reload() noexcept { _start = now(); }

    void poll(auto&& action) noexcept    // TODO find proper name
    {
        if (!isElapsed()) {
            return;
        }
        reload();
        action();
    }

  private:
    SYS_TIM_T::TickType now() const noexcept { return _sysTim.getTicks(); }    // TODO infer return type?

    SYS_TIM_T::TickType _start;
    const SYS_TIM_T::TickType _period;
    const SYS_TIM_T& _sysTim;
};

#endif    // TIMER_HPP
