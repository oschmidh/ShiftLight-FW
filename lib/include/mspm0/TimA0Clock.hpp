#ifndef LIB_INCLUDE_MSPM0_TIMA0CLOCK_HPP
#define LIB_INCLUDE_MSPM0_TIMA0CLOCK_HPP

#include "ti_msp_dl_config.h"

#include <limits>
#include <cstdint>

class PeriodicTimer {
  public:
    static constexpr auto intLine = std::integral_constant<unsigned int, TIMA0_INT_IRQn>{};

    static constexpr unsigned int presc = 255;             // TODO hardcoded here
    static constexpr unsigned int clkFreq = 24'000'000;    // TODO hardcoded here

    using CallbackType = void (*)(void);

    using TickType = std::uint16_t;
    static constexpr TickType period = std::numeric_limits<TickType>::max();

    void init(CallbackType elapsedCallback) noexcept
    {
        _cb = elapsedCallback;

        DL_TimerA_reset(TIMA0);
        DL_TimerA_enablePower(TIMA0);

        constexpr DL_TimerA_ClockConfig clkCfg{
            .clockSel = DL_TIMER_CLOCK_BUSCLK, .divideRatio = DL_TIMER_CLOCK_DIVIDE_1, .prescale = presc};
        DL_TimerA_setClockConfig(TIMA0, &clkCfg);

        constexpr DL_TimerA_TimerConfig timerCfg = {
            .timerMode = DL_TIMER_TIMER_MODE_PERIODIC_UP,
            .period = period,
            .startTimer = DL_TIMER_STOP,
            .genIntermInt = DL_TIMER_INTERM_INT_DISABLED,
            .counterVal = 0,
        };
        DL_TimerA_initTimerMode(TIMA0, &timerCfg);

        DL_TimerA_enableInterrupt(TIMA0, DL_TIMERA_INTERRUPT_LOAD_EVENT);
        DL_TimerA_enableClock(TIMA0);
        DL_TimerA_setCoreHaltBehavior(TIMA0, DL_TIMER_CORE_HALT_IMMEDIATE);    // TODO ??

        NVIC_EnableIRQ(TIMA0_INT_IRQn);
        DL_TimerA_startCounter(TIMA0);
    }

    TickType getTicks() noexcept { return DL_TimerA_getTimerCount(TIMA0); }

    void isr()
    {
        switch (DL_TimerA_getPendingInterrupt(TIMA0)) {
            case DL_TIMERG_IIDX_LOAD:
                if (_cb) {
                    _cb();
                }
                break;
            default: break;
        }
    }

  private:
    CallbackType _cb;
};

#endif    // LIB_INCLUDE_MSPM0_TIMA0CLOCK_HPP
