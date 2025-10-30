#include "ti_msp_dl_config.h"

#include "Clock.hpp"

#include <chrono>
#include <limits>
#include <cstdint>

namespace System {

class CounterTimA {
  public:
    static constexpr unsigned int presc = 255;             // TODO hardcoded here
    static constexpr unsigned int clkFreq = 24'000'000;    // TODO hardcoded here

    using TickType = std::uint16_t;
    static constexpr TickType period = std::numeric_limits<TickType>::max();
    // using DurationType = std::chrono::duration<TickType, std::ratio<presc, timClk>>;

    // constexpr CounterTimA() noexcept { }

    static void init() noexcept
    {
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

    // void enable() noexcept { DL_TimerA_startCounter(TIMA0); }
    // void disable() noexcept { DL_TimerA_stopCounter(TIMA0); }
    // static TickType getPeriod() noexcept { return DL_TimerA_getLoadValue(TIMA0); }
    static TickType getTicks() noexcept { return DL_TimerA_getTimerCount(TIMA0); }

    // template <typename REP_T, typename PERIOD_T>
    // static constexpr TickType toTicks(std::chrono::duration<REP_T, PERIOD_T> duration) noexcept
    // {
    //     return std::chrono::duration_cast<DurationType>(duration).count();
    // }
};

using SteadyClock = TimerSteadyClock<CounterTimA>;

extern "C" void TIMA0_IRQHandler(void)    // TODO should be part of CounterTimA
{
    switch (DL_TimerA_getPendingInterrupt(TIMA0)) {
        case DL_TIMERG_IIDX_LOAD: SteadyClock::overflowIsr(); break;
        default: break;
    }
}

// template <typename REP_T, typename PERIOD_T>
// static void busyWait(std::chrono::duration<REP_T, PERIOD_T> duration) noexcept
// {
//    using MainClkDuration= std::chrono::duration<unsigned int, std::ratio<1, 24'000'000>>;
//     delay_cycles(std::chrono::duration_cast<MainClkDuration>(duration).count());
// }

template <typename REP_T, typename PERIOD_T>
static void busyWait(std::chrono::duration<REP_T, PERIOD_T> duration) noexcept
{
    const auto start = SteadyClock::now();
    while (SteadyClock::now() < (start + duration))
        ;
}

}    // namespace System
