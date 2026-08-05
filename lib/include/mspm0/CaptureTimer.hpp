#ifndef LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP
#define LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP

#include "Timer.hpp"

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
    unsigned int prescaler;
};

template <CaptureTimerConfig CFG_V>
class CaptureTimer {
  public:
    using ErrorType = CaptureTimerError;

    static constexpr auto intLine = std::integral_constant<unsigned int, CFG_V.intLine>{};

    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here
    static constexpr unsigned int prescaler = CFG_V.prescaler;
    static_assert(prescaler <= 0xff);

    constexpr CaptureTimer(Timer& tim) noexcept
     : _tim(tim)
    { }

    void init() noexcept
    {
        _tim.init(prescaler);

        _tim.setReloadVal(0xffff);

        _tim.configure({.countMode = Timer::CountMode::Up,
                        .repeat = Timer::Repeat::Yes,
                        .ctrLoadControl = CFG_V.channel,
                        .ctrAdvanceControl = CFG_V.channel,
                        .ctrZeroControl = CFG_V.channel,
                        .ctrValAfterEn = Timer::CtrValAfterEn::LoadVal});

        // automatic load must be disabled, because the load seems to happen before the captured value is transferred.
        // Therefore the capture register would always contain the load value (see ERRATA TIMER_ERR_01)
        _tim.configureCcpChannel(CFG_V.channel, {.captureCondition = Timer::CaptureCondition::FallingEdge,
                                                 .advanceCondition = Timer::AdvanceCondition::TimerClk,
                                                 .loadCondition = Timer::LoadCondition::None,
                                                 .zeroCondition = Timer::ZeroCondition::None,
                                                 .captureOrCompare = Timer::CaptureOrCompare::Capture});

        _tim.configureCcpDirection(CFG_V.channel, Timer::CcpDirection::Input);

        _tim.enableInterrupts(Timer::Interrupts::captureCompareUp[CFG_V.channel], Timer::Interrupts::Overflow);

        _tim.enableClock();
    }

    void enable() noexcept
    {
        _tim.enableInterruptLine(CFG_V.intLine);
        _tim.start();
    }

    std::expected<std::uint32_t, ErrorType> getPeriod() const noexcept
    {
        if (!_synced) {
            return std::unexpected(CaptureTimerError::NotSynced);
        }

        return _tim.getCcpValue(CFG_V.channel);
    }

    void isr() noexcept
    {
        const auto pending = _tim.getNextPendingInterrupt();
        if (!pending) {
            return;
        }

        switch (*pending) {
            case Timer::Interrupts::captureCompareUp[CFG_V.channel]:
                _synced = true;
                _tim.setCounter(0);    // Manual reload, workaround for ERRATA TIMER_ERR_01
                break;
            case Timer::Interrupts::Overflow:
                _synced = false;
                break;
            default:
                break;
        }
    }

  private:
    Timer& _tim;
    volatile bool _synced{};
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP
