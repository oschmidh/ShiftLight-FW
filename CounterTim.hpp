#ifndef COUNTERTIM_HPP
#define COUNTERTIM_HPP

#include "ti_msp_dl_config.h"

#include <cstdint>

class CounterTimA {
  public:
    using TickType = std::uint32_t;

    static constexpr unsigned int presc = 255;            // TODO hardcoded here
    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here

    constexpr CounterTimA() noexcept { }

    void init() noexcept
    {
        DL_TimerA_reset(TIMA0);
        DL_TimerA_enablePower(TIMA0);

        constexpr DL_TimerA_ClockConfig clkCfg{
            .clockSel = DL_TIMER_CLOCK_BUSCLK, .divideRatio = DL_TIMER_CLOCK_DIVIDE_1, .prescale = presc};
        DL_TimerA_setClockConfig(TIMA0, &clkCfg);

        constexpr DL_TimerA_TimerConfig timerCfg = {
            .timerMode = DL_TIMER_TIMER_MODE_PERIODIC_UP,
            .period = 0xffffff,    // TODO use max?
            .startTimer = DL_TIMER_STOP,
            .genIntermInt = DL_TIMER_INTERM_INT_DISABLED,
            .counterVal = 0,
        };
        DL_TimerA_initTimerMode(TIMA0, &timerCfg);

        DL_TimerA_enableClock(TIMA0);
        DL_TimerA_setCoreHaltBehavior(TIMA0, DL_TIMER_CORE_HALT_IMMEDIATE);    // TODO ??
    }

    void enable() noexcept { DL_TimerA_startCounter(TIMA0); }
    void disable() noexcept { DL_TimerA_stopCounter(TIMA0); }
    TickType getPeriod() const noexcept { return DL_TimerA_getLoadValue(TIMA0); }
    TickType getTicks() const noexcept { return DL_TimerA_getTimerCount(TIMA0); }
};

#endif    // COUNTERTIM_HPP
