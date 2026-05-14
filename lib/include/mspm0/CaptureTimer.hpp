#ifndef LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP
#define LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP

#include "BaseTimer.hpp"

#include <chrono>
#include <expected>
#include <cstdint>

namespace mspm0 {

enum class CaptureTimerError {
    NoError,
    NotSynced,
};

struct CaptureTimerConfig {
    unsigned int intLine;
    unsigned int channel;
    unsigned int prescaler;    // TODO max 0xff -> check somewhere?
};

template <CaptureTimerConfig CFG_V>
class CaptureTimer {
  public:
    using ErrorType = CaptureTimerError;

    static constexpr auto intLine = std::integral_constant<unsigned int, CFG_V.intLine>{};

    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here

    constexpr CaptureTimer(uintptr_t addr) noexcept
     : _tim(addr)
    { }

    void init() noexcept
    {
        _tim.init(CFG_V.prescaler);

        _tim.setReloadVal(0xffff);
        // static_assert((1 << CFG_V.resolution) - 1 == 0xffff);

        _tim.configure({.countMode = BaseTimer::CountMode::Up,
                        .repeat = BaseTimer::Repeat::Yes,
                        .ctrLoadControl = CFG_V.channel,
                        .ctrAdvanceControl = CFG_V.channel,
                        .ctrZeroControl = CFG_V.channel,
                        .ctrValAfterEn = BaseTimer::CtrValAfterEn::Zero});

        // automatic load must be disabled, because the load seems to happen before the captured value is transferred.
        // Therefore the capture register would always contain the load value (see ERRATA TIMER_ERR_01)
        _tim.getCcpChannel(CFG_V.channel)
            .configure({.captureCondition = BaseTimer::CcpChannel::CaptureCondition::FallingEdge,
                        .advanceCondition = BaseTimer::CcpChannel::AdvanceCondition::TimerClk,
                        .loadCondition = BaseTimer::CcpChannel::LoadCondition::None,
                        .zeroCondition = BaseTimer::CcpChannel::ZeroCondition::None,
                        .captureOrCompare = BaseTimer::CcpChannel::CaptureOrCompare::Capture});

        _tim.configureCcpDirection(CFG_V.channel, BaseTimer::CcpDirection::Input);

        _tim.enableInterrupts(DL_TIMERG_INTERRUPT_CC1_UP_EVENT | DL_TIMERG_INTERRUPT_OVERFLOW_EVENT);

        _tim.enableClock();
    }

    void enable() noexcept
    {
        NVIC_EnableIRQ(static_cast<IRQn_Type>(CFG_V.intLine));    // TODO remove cast
        _tim.start();
    }

    using PeriodType = std::chrono::duration<std::uint32_t, std::ratio<(CFG_V.prescaler + 1), timClk>>;

    std::expected<PeriodType, ErrorType> getPeriod() const noexcept
    {
        if (!_synced) {
            return std::unexpected(CaptureTimerError::NotSynced);
        }

        return PeriodType{_tim.getCcpChannel(CFG_V.channel).getValue()};
    }

    void isr() noexcept
    {
        switch (_tim.getPendingInterrupts()) {
            case DL_TIMERG_IIDX_CC1_UP:
                _synced = true;
                _tim.setCounter(0);    // Manual reload, workaround for ERRATA TIMER_ERR_01
                // _callback(PeriodType{_ctrRegs->CC[CFG_V.channel]}); // TODO implement
                break;
            case DL_TIMERG_IIDX_OVERFLOW:
                /* If Timer reaches overflows then no PWM signal is detected and it
                 * requires re-synchronization
                 */
                _synced = false;
                break;
            default: break;
        }
    }

  private:
    BaseTimer _tim;
    volatile bool _synced{};
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP
