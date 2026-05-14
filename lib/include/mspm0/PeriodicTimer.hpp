#ifndef LIB_INCLUDE_MSPM0_PERIODICTIMER_HPP
#define LIB_INCLUDE_MSPM0_PERIODICTIMER_HPP

#include "BaseTimer.hpp"

#include <limits>
#include <cstdint>

namespace mspm0 {

struct PeriodicTimerConfig {
    unsigned int intLine;
    unsigned int channel;
    unsigned int prescaler;    // TODO max 0xff -> check somewhere?
};

template <PeriodicTimerConfig CFG_V>
class PeriodicTimer {
  public:
    static constexpr auto intLine = std::integral_constant<unsigned int, TIMA0_INT_IRQn>{};

    static constexpr unsigned int presc = CFG_V.prescaler;
    static constexpr unsigned int clkFreq = 24'000'000;    // TODO hardcoded here

    using CallbackType = void (*)(void);

    using TickType = std::uint16_t;
    static constexpr TickType period = std::numeric_limits<TickType>::max();

    constexpr PeriodicTimer(uintptr_t addr) noexcept
     : _tim(addr)
    { }

    void init(CallbackType elapsedCallback) noexcept
    {
        _cb = elapsedCallback;

        _tim.init(presc);

        // constexpr DL_TimerA_ClockConfig clkCfg{
        //     .clockSel = DL_TIMER_CLOCK_BUSCLK, .divideRatio = DL_TIMER_CLOCK_DIVIDE_1, .prescale = presc};
        // DL_TimerA_setClockConfig(TIMA0, &clkCfg);

        _tim.setReloadVal(period);

        _tim.configure({.countMode = BaseTimer::CountMode::Up,
                        .repeat = BaseTimer::Repeat::Yes,
                        .ctrLoadControl = CFG_V.channel,
                        .ctrAdvanceControl = CFG_V.channel,
                        .ctrZeroControl = CFG_V.channel,
                        .ctrValAfterEn = BaseTimer::CtrValAfterEn::Zero});

        _tim.setCaptureCompareVal(CFG_V.channel, 0);
        _tim.configureCaptureCompare({.channel = CFG_V.channel,
                                      .advanceCondition = BaseTimer::AdvanceCondition::TimerClk,
                                      .captureOrCompare = BaseTimer::CaptureOrCompare::Capture});

        _tim.enableInterrupts(DL_TIMERA_INTERRUPT_LOAD_EVENT);
        _tim.enableClock();
        // DL_TimerA_setCoreHaltBehavior(TIMA0, DL_TIMER_CORE_HALT_IMMEDIATE);    // TODO ??

        NVIC_EnableIRQ(TIMA0_INT_IRQn);
        _tim.start();
    }

    TickType getTicks() noexcept { return DL_TimerA_getTimerCount(TIMA0); }

    void isr()
    {
        switch (_tim.getPendingInterrupts()) {
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
    BaseTimer _tim;
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_PERIODICTIMER_HPP
