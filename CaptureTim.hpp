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

class CaptureTimG {
  public:
    static constexpr unsigned int presc = 255;            // TODO hardcoded here
    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here

    constexpr CaptureTimG() noexcept { }

    void init() noexcept
    {
        DL_TimerG_reset(TIMG8);

        DL_TimerG_enablePower(TIMG8);

        constexpr DL_TimerG_ClockConfig clkCfg{
            .clockSel = DL_TIMER_CLOCK_BUSCLK, .divideRatio = DL_TIMER_CLOCK_DIVIDE_1, .prescale = presc};
        DL_TimerG_setClockConfig(TIMG8, &clkCfg);

        // TODO should only use pulseWidth capture mode?
        constexpr DL_TimerG_CaptureCombinedConfig captureCfg = {
            .captureMode = DL_TIMER_CAPTURE_COMBINED_MODE_PULSE_WIDTH_AND_PERIOD,
            .period = 49151,    // TODO??
            .startTimer = DL_TIMER_STOP,
            .inputChan = DL_TIMER_INPUT_CHAN_1,
            .inputInvMode = DL_TIMER_CC_INPUT_INV_NOINVERT,
        };

        DL_TimerG_initCaptureCombinedMode(TIMG8, &captureCfg);
        DL_TimerG_enableInterrupt(TIMG8, DL_TIMERG_INTERRUPT_CC1_DN_EVENT | DL_TIMERG_INTERRUPT_ZERO_EVENT);

        DL_TimerG_enableClock(TIMG8);
    }

    void enable() noexcept
    {
        gLoadValue = DL_TimerG_getLoadValue(TIMG8);    // TODO ??

        DL_TimerG_setCoreHaltBehavior(TIMG8, DL_TIMER_CORE_HALT_IMMEDIATE);    // TODO ??

        NVIC_EnableIRQ(TIMG8_INT_IRQn);
        DL_TimerG_startCounter(TIMG8);
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

extern "C" void TIMG8_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMG8)) {
        case DL_TIMERG_IIDX_CC1_DN:
            if (gSynced == true) {
                gCaptureCnt = DL_TimerG_getCaptureCompareValue(TIMG8, DL_TIMER_CC_1_INDEX);
                gCheckCaptures = true;
            } else {
                gSynced = true;
            }
            /* Manual reload is needed to workaround timer capture limitation */
            DL_TimerG_setTimerCount(TIMG8, gLoadValue);
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
