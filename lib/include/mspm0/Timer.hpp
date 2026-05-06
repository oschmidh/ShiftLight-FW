#ifndef LIB_INCLUDE_MSPM0_TIMER_HPP
#define LIB_INCLUDE_MSPM0_TIMER_HPP

#include "RegSet.hpp"

#include <utility>
#include <cstdint>

namespace mspm0 {

struct TimerConfig {
    unsigned int intLine;
    unsigned int channel;
    unsigned int prescaler;    // TODO max 0xff -> check somewhere?
};

template <TimerConfig CFG_V>    // TODO add base class to avoid template bloat
class Timer {
  public:
    enum class CcpDirection { Input = 0, Output = 1 };

    constexpr Timer(uintptr_t addr) noexcept
     : _pwrCtrl(addr)
     , _clkCtrl(addr)
     , _intCtrl(addr, detail::regSet::intRegOffset)
     , _commonRegs(new (reinterpret_cast<std::uint32_t*>(addr + commonRegOffset)) CommonRegisters)
     , _ctrRegs(new (reinterpret_cast<std::uint32_t*>(addr + ctrRegOffset)) CounterRegisters)
    { }

    void init() noexcept
    {
        _pwrCtrl.reset();
        _pwrCtrl.enable();

        _clkCtrl.setSource(detail::regSet::ClockControl::ClockSource::BusClk);
        _commonRegs->CPS = CFG_V.prescaler;
    }

    void setReloadVal(std::uint32_t cntrVal) noexcept { _ctrRegs->LOAD = cntrVal; }

    // static_assert((1 << CFG_V.resolution) - 1 == 0xffff);

    enum class CountMode : std::uint32_t {
        Down = 0,
        UpDown = 1,
        Up = 2,
    };

    enum class Repeat : std::uint32_t {
        No = 0,
        Yes = 1,
        // reserved = 2
        StopInDebug = 3,
    };

    enum class CtrValAfterEn : std::uint32_t {
        LoadVal = 0,
        Unchanged = 1,
        Zero = 2,
    };

    // enum class CtrLoadControl : std::uint32_t {
    // LoadVal=0,
    // Unchanged=1,
    // Zero=2,
    // };

    struct Config {
        CountMode countMode = CountMode::Down;
        Repeat repeat = Repeat::No;
        std::uint32_t ctrLoadControl = 7;       // TODO use enum?
        std::uint32_t ctrAdvanceControl = 7;    // TODO use enum?
        std::uint32_t ctrZeroControl = 7;       // TODO use enum?
        bool phaseLoadEn = false;
        CtrValAfterEn ctrValAfterEn = CtrValAfterEn::LoadVal;
    };

    enum class CaptureCondition : std::uint32_t {
        None = 0,
        RisingEdge = 1,
        FallingEdge = 2,
        BothEdges = 3,
    };

    enum class AdvanceCondition : std::uint32_t {
        TimerClk = 0,
        RisingEdge = 1,
        FallingEdge = 2,
        BothEdges = 3,
        // reserved = 4
        HighLevel = 5,
    };

    enum class LoadCondition : std::uint32_t {
        None = 0,
        RisingEdge = 1,
        FallingEdge = 2,
        BothEdges = 3,
    };

    enum class ZeroCondition : std::uint32_t {
        None = 0,
        RisingEdge = 1,
        FallingEdge = 2,
        BothEdges = 3,
    };

    enum class CaptureOrCompare : std::uint32_t {
        Compare = 0,
        Capture = 1,
    };

    struct CaptureCompareConfig {
        unsigned int channel;
        CaptureCondition captureCondition = CaptureCondition::None;
        AdvanceCondition advanceCondition = AdvanceCondition::TimerClk;
        LoadCondition loadCondition = LoadCondition::None;
        ZeroCondition zeroCondition = ZeroCondition::None;
        CaptureOrCompare captureOrCompare = CaptureOrCompare::Compare;
    };

    void configure(const Config& cfg) noexcept
    {
        _ctrRegs->CTRCTL = (std::to_underlying(cfg.ctrValAfterEn) << 28u) | (cfg.phaseLoadEn << 24u) |
                           (cfg.ctrZeroControl << 13u) | (cfg.ctrAdvanceControl << 10u) | (cfg.ctrLoadControl << 7u) |
                           (std::to_underlying(cfg.countMode) << 4u) |
                           (std::to_underlying(cfg.repeat) << 1u);    // TODO magic
                                                                      // offsets
    }

    void configureCaptureCompare(const CaptureCompareConfig& cfg) noexcept
    {
        // TODO verify channel in range?
        _ctrRegs->CCCTL[cfg.channel] =
            std::to_underlying(cfg.captureOrCompare) | (std::to_underlying(cfg.captureOrCompare) << 17u) |
            (std::to_underlying(cfg.zeroCondition) << 12u) | (std::to_underlying(cfg.loadCondition) << 8u) |
            (std::to_underlying(cfg.advanceCondition) << 4u) | (std::to_underlying(cfg.captureCondition));    // TODO
                                                                                                              // magic
                                                                                                              // offsets
    }

    void configureCcpDirection(unsigned int channel, CcpDirection dir) noexcept
    {
        // TODO verify channel in range?
        _commonRegs->CCPD &= ~(1u << channel);
        _commonRegs->CCPD |= std::to_underlying(dir) << channel;
    }

    std::uint32_t getCaptureCompareVal(unsigned int channel) const noexcept { return _ctrRegs->CC[channel]; }

    void setCaptureCompareVal(unsigned int channel, std::uint32_t value) const noexcept
    {
        _ctrRegs->CC[channel] = value;
    }

    void setCounter(std::uint32_t val) noexcept { _ctrRegs->CTR = val; }

    void enableClock() noexcept { _commonRegs->CCLKCTL |= 1; }

    void disableClock() noexcept { _commonRegs->CCLKCTL &= ~1; }

    void enableInterrupts(std::uint32_t mask) noexcept
    {
        _intCtrl.enableInterrupts(DL_TIMERG_INTERRUPT_CC1_UP_EVENT | DL_TIMERG_INTERRUPT_OVERFLOW_EVENT);
    }

    std::uint32_t getPendingInterrupts() const noexcept { return _intCtrl.getPending(); }

    void start() noexcept
    {
        _ctrRegs->CTRCTL |= 1;    // TODO magic number
    }

    void stop() noexcept
    {
        _ctrRegs->CTRCTL &= ~1;    // TODO magic number
    }

  private:
    struct CommonRegisters {
        std::uint32_t CCPD;
        std::uint32_t ODIS;
        std::uint32_t CCLKCTL;
        std::uint32_t CPS;
        std::uint32_t CPSV;
        std::uint32_t CTTTRIGCTRL;
        std::uint32_t reserved;
        std::uint32_t CTTTRIG;
    };

    struct CounterRegisters {
        std::uint32_t CTR;
        std::uint32_t CTRCTL;
        std::uint32_t LOAD;
        std::uint32_t reserved_0;
        std::uint32_t CC[6];
        std::uint32_t reserved_1[2];
        std::uint32_t CCCTL[6];
        std::uint32_t reserved_2[2];
        std::uint32_t OCTL[4];
        std::uint32_t reserved_3[4];
        std::uint32_t CCACT[4];
        std::uint32_t IFCTL[4];
    };

    detail::regSet::PowerControl _pwrCtrl;
    detail::regSet::ClockControl _clkCtrl;
    detail::regSet::InterruptControl _intCtrl;

    static constexpr uintptr_t commonRegOffset = 0x1100;
    static constexpr uintptr_t ctrRegOffset = 0x1800;

    volatile CommonRegisters* const _commonRegs;
    volatile CounterRegisters* const _ctrRegs;

    // fn_ref _callback;    // TODO implement
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_TIMER_HPP
