#ifndef CAPTURETIM_HPP
#define CAPTURETIM_HPP

// #include <ti/driverlib/driverlib.h>
#include "ti_msp_dl_config.h"

#include <chrono>
#include <cstdint>

// TODO integrate in class
volatile uint32_t gCaptureCnt;
volatile bool gSynced;
volatile bool gCheckCaptures;
uint32_t gLoadValue;

class CaptureTimA {
  public:
    static constexpr unsigned int presc = 255;            // TODO hardcoded here
    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here

    constexpr CaptureTimA() noexcept { }

    void init() noexcept
    {
        DL_TimerA_reset(TIMA0);

        DL_TimerA_enablePower(TIMA0);

        constexpr DL_TimerA_ClockConfig clkCfg{
            .clockSel = DL_TIMER_CLOCK_BUSCLK, .divideRatio = DL_TIMER_CLOCK_DIVIDE_1, .prescale = presc};
        DL_TimerA_setClockConfig(TIMA0, &clkCfg);

        // TODO should only use pulseWidth capture mode?
        constexpr DL_TimerA_CaptureCombinedConfig captureCfg = {
            .captureMode = DL_TIMER_CAPTURE_COMBINED_MODE_PULSE_WIDTH_AND_PERIOD,
            .period = 49151,    // TODO??
            .startTimer = DL_TIMER_STOP,
            .inputChan = DL_TIMER_INPUT_CHAN_0,
            .inputInvMode = DL_TIMER_CC_INPUT_INV_INVERT,
        };

        DL_TimerA_initCaptureCombinedMode(TIMA0, &captureCfg);
        DL_TimerA_enableInterrupt(TIMA0, DL_TIMERA_INTERRUPT_CC1_DN_EVENT | DL_TIMERA_INTERRUPT_ZERO_EVENT);

        DL_TimerA_enableClock(TIMA0);
    }

    void enable() noexcept
    {
        gLoadValue = DL_TimerG_getLoadValue(TIMA0);    // TODO ??

        DL_TimerA_setCoreHaltBehavior(TIMA0, DL_TIMER_CORE_HALT_IMMEDIATE);    // TODO ??

        NVIC_EnableIRQ(TIMA0_INT_IRQn);
        DL_TimerA_startCounter(TIMA0);
    }

    using PeriodType = std::chrono::duration<std::uint32_t, std::ratio<(presc + 1), timClk>>;

    PeriodType getPeriod() const noexcept
    {
        while (!gCheckCaptures) {
            __WFE();
        }
        gCheckCaptures = false;

        return PeriodType{gLoadValue - gCaptureCnt};
    }

  private:
};

extern "C" void TIMA0_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIMA0)) {
        case DL_TIMERG_IIDX_CC1_DN:
            if (gSynced == true) {
                gCaptureCnt = DL_TimerA_getCaptureCompareValue(TIMA0, DL_TIMER_CC_1_INDEX);
                gCheckCaptures = true;
            } else {
                gSynced = true;
            }
            /* Manual reload is needed to workaround timer capture limitation */
            DL_TimerG_setTimerCount(TIMA0, gLoadValue);
            break;
        case DL_TIMERG_IIDX_ZERO:
            /* If Timer reaches zero then no PWM signal is detected and it
             * requires re-synchronization
             */
            gSynced = false;
            break;
        default: break;
    }
}

#endif
