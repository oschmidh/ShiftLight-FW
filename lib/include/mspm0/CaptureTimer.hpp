#ifndef LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP
#define LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP

#include "Timer.hpp"

#include <chrono>
#include <expected>
#include <cstdint>

// static constexpr unsigned int dynamicFreq = std::numeric_limits<unsigned int>::max();

// template <unsigned int CLK_FREQ_V = dynamicFreq>
// class ClockSource {
//   public:
//   private:
// };

namespace mspm0 {

enum class CaptureTimerError {
    NoError,
    NotSynced,
};

// template <typename CLK_SRC_T>
struct CaptureTimerConfig {
    // CLK_SRC_T clkSrc;
    unsigned int intLine;
    unsigned int channel;
    unsigned int prescaler;
};

// template <typename CLK_SRC_T, CaptureTimerConfig CFG_V>
template <CaptureTimerConfig CFG_V>
// template <auto CFG_V>
class CaptureTimer {
  public:
    using ErrorType = CaptureTimerError;

    static constexpr auto intLine = std::integral_constant<unsigned int, CFG_V.intLine>{};

    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here
    static_assert(CFG_V.prescaler <= 0xff);

    constexpr CaptureTimer(Timer& tim) noexcept
     : _tim(tim)
    { }

    void init() noexcept
    {
        _tim.init(CFG_V.prescaler);

        _tim.setReloadVal(0xffff);
        // static_assert((1 << CFG_V.resolution) - 1 == 0xffff);

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
        // cortex_m0::nvic::enableInterrupt(CFG_V.intLine);
        _tim.enableInterruptLine(CFG_V.intLine);
        _tim.start();
    }

    using PeriodType = std::chrono::duration<std::uint32_t, std::ratio<(CFG_V.prescaler + 1), timClk>>;

    std::expected<PeriodType, ErrorType> getPeriod() const noexcept
    {
        if (!_synced) {
            return std::unexpected(CaptureTimerError::NotSynced);
        }

        return PeriodType{_tim.getCcpValue(CFG_V.channel)};
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
            default: break;
        }
    }

  private:
    Timer& _tim;
    volatile bool _synced{};
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_CAPTURETIMER_HPP
