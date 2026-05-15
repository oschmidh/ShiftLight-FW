#ifndef LIB_INCLUDE_MSPM0_PERIODICTIMER_HPP
#define LIB_INCLUDE_MSPM0_PERIODICTIMER_HPP

#include "Timer.hpp"

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

        _tim.configure({.countMode = Timer::CountMode::Up,
                        .repeat = Timer::Repeat::Yes,
                        .ctrLoadControl = CFG_V.channel,
                        .ctrAdvanceControl = CFG_V.channel,
                        .ctrZeroControl = CFG_V.channel,
                        .ctrValAfterEn = Timer::CtrValAfterEn::Zero});

        _tim.getCcpChannel(CFG_V.channel).setValue(0);
        _tim.getCcpChannel(CFG_V.channel)
            .configure({.advanceCondition = Timer::CcpChannel::AdvanceCondition::TimerClk,
                        .captureOrCompare = Timer::CcpChannel::CaptureOrCompare::Capture});

        _tim.enableInterrupts(Timer::Interrupts::Load);
        _tim.enableClock();
        // DL_TimerA_setCoreHaltBehavior(TIMA0, DL_TIMER_CORE_HALT_IMMEDIATE);    // TODO ??

        NVIC_EnableIRQ(static_cast<IRQn_Type>(CFG_V.intLine));
        _tim.start();
    }

    TickType getTicks() noexcept { return DL_TimerA_getTimerCount(TIMA0); }

    void isr()
    {
        const auto pending = _tim.getNextPendingInterrupt();
        if (!pending) {
            return;
        }

        switch (*pending) {
            case Timer::Interrupts::Load:
                if (_cb) {
                    _cb();
                }
                break;
            default: break;
        }
    }

  private:
    CallbackType _cb;
    Timer _tim;
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_PERIODICTIMER_HPP
