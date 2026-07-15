#ifndef LIB_INCLUDE_MSPM0_TIMER_HPP
#define LIB_INCLUDE_MSPM0_TIMER_HPP

#include "CommonRegs.hpp"

#include <utility>
#include <optional>
#include <cstdint>

namespace mspm0 {

class Timer : detail::Peripheral<Timer> {
  public:
    enum class ClockSource : std::uint32_t {
        BusClk = 1 << 3u,
        MfClk = 1 << 2u,
        LfClk = 1 << 1u,
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
        CaptureCondition captureCondition = CaptureCondition::None;
        AdvanceCondition advanceCondition = AdvanceCondition::TimerClk;
        LoadCondition loadCondition = LoadCondition::None;
        ZeroCondition zeroCondition = ZeroCondition::None;
        CaptureOrCompare captureOrCompare = CaptureOrCompare::Compare;
    };

    enum class CcpDirection { Input = 0, Output = 1 };

    constexpr Timer(std::uintptr_t addr) noexcept
     : detail::Peripheral<Timer>(addr)
     //  , _intCtrl(addr)
     , _clkRegs(new (reinterpret_cast<std::uint32_t*>(addr + clockRegOffset)) ClockRegisters)
     , _commonRegs(new (reinterpret_cast<std::uint32_t*>(addr + commonRegOffset)) CommonRegisters)
     , _ctrRegs(new (reinterpret_cast<std::uint32_t*>(addr + ctrRegOffset)) CounterRegisters)
    { }

    void init(std::uint8_t prescaler) noexcept
    {
        _pwrRegs->resetCtl.reset();
        _pwrRegs->powerEn.enable();

        _clkRegs->clockSel.setSource(ClockSource::BusClk);
        _commonRegs->CPS = prescaler;
    }

    void setReloadVal(std::uint32_t cntrVal) noexcept { _ctrRegs->LOAD = cntrVal; }    // TODO should be uint16?

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

    void configure(const Config& cfg) noexcept
    {
        _ctrRegs->CTRCTL = (std::to_underlying(cfg.ctrValAfterEn) << 28u) | (cfg.phaseLoadEn << 24u) |
                           (cfg.ctrZeroControl << 13u) | (cfg.ctrAdvanceControl << 10u) | (cfg.ctrLoadControl << 7u) |
                           (std::to_underlying(cfg.countMode) << 4u) |
                           (std::to_underlying(cfg.repeat) << 1u);    // TODO magic
                                                                      // offsets
    }

    void configureCcpChannel(unsigned int channel, const CaptureCompareConfig& cfg) noexcept
    {
        _ctrRegs->CCCTL[channel] =
            (std::to_underlying(cfg.captureOrCompare) << 17u) | (std::to_underlying(cfg.zeroCondition) << 12u) |
            (std::to_underlying(cfg.loadCondition) << 8u) | (std::to_underlying(cfg.advanceCondition) << 4u) |
            std::to_underlying(cfg.captureCondition);    // TODO magic offsets
    }

    void configureCcpDirection(unsigned int channel, CcpDirection dir) noexcept    // TODO add enum for channel
    {
        // TODO verify channel in range?
        _commonRegs->CCPD &= ~(1u << channel);
        _commonRegs->CCPD |= std::to_underlying(dir) << channel;
    }

    std::uint32_t getCcpValue(unsigned int channel) const noexcept { return _ctrRegs->CC[channel]; }

    void setCcpValue(unsigned int channel, std::uint32_t val) noexcept { _ctrRegs->CC[channel] = val; }

    void setCounter(std::uint16_t val) noexcept { _ctrRegs->CTR = val; }
    std::uint16_t getCounter() const noexcept { return _ctrRegs->CTR; }

    void enableClock() noexcept { _commonRegs->CCLKCTL |= 1; }

    void disableClock() noexcept { _commonRegs->CCLKCTL &= ~1; }

    struct Interrupts {
        enum class InterruptVals : std::uint32_t {
            Zero = 0,
            Load = 1,
            CaptureCompareDown0 = 4,
            CaptureCompareDown1 = 5,
            CaptureCompareDown2 = 6,
            CaptureCompareDown3 = 7,
            CaptureCompareUp0 = 8,
            CaptureCompareUp1 = 9,
            CaptureCompareUp2 = 10,
            CaptureCompareUp3 = 11,
            CaptureCompareUp4 = 12,
            CaptureCompareUp5 = 13,
            Fault = 24,
            Overflow = 25,
            RepeatCntZero = 26,
            DirectionChange = 27,
            Qeierr = 28,
        };

        using enum InterruptVals;

        static constexpr std::array captureCompareUp{
            InterruptVals::CaptureCompareUp0, InterruptVals::CaptureCompareUp1, InterruptVals::CaptureCompareUp2,
            InterruptVals::CaptureCompareUp3, InterruptVals::CaptureCompareUp4, InterruptVals::CaptureCompareUp5};
        static constexpr std::array captureCompareDown{
            InterruptVals::CaptureCompareDown0, InterruptVals::CaptureCompareDown1, InterruptVals::CaptureCompareDown2,
            InterruptVals::CaptureCompareDown3};
    };

    template <typename... Ts>
        requires(std::is_same_v<Ts, Interrupts::InterruptVals> && ...)
    void enableInterrupts(Ts... interrupts) noexcept
    {

        _intEvRegs->intCtrl.enable(interrupts...);
    }

    std::optional<Interrupts::InterruptVals> getNextPendingInterrupt() const noexcept
    {

        return _intEvRegs->intCtrl.getNextPending();
    }

    void start() noexcept
    {
        _ctrRegs->CTRCTL |= 1;    // TODO magic number
    }

    void stop() noexcept
    {
        _ctrRegs->CTRCTL &= ~1;    // TODO magic number
    }

  private:
    struct ClockRegisters {
        detail::commonRegs::ClockDiv clockDiv;
        std::uint32_t reserved;
        detail::commonRegs::ClockSel<ClockSource> clockSel;
    };

    struct CommonRegisters {
        volatile std::uint32_t CCPD;
        volatile std::uint32_t ODIS;
        volatile std::uint32_t CCLKCTL;
        volatile std::uint32_t CPS;
        volatile std::uint32_t CPSV;
        volatile std::uint32_t CTTTRIGCTRL;
        volatile std::uint32_t reserved;
        volatile std::uint32_t CTTTRIG;
    };

    struct CounterRegisters {
        volatile std::uint32_t CTR;
        volatile std::uint32_t CTRCTL;
        volatile std::uint32_t LOAD;
        volatile std::uint32_t reserved_0;
        volatile std::uint32_t CC[6];
        volatile std::uint32_t reserved_1[2];
        volatile std::uint32_t CCCTL[6];
        volatile std::uint32_t reserved_2[2];
        volatile std::uint32_t OCTL[4];
        volatile std::uint32_t reserved_3[4];
        volatile std::uint32_t CCACT[4];
        volatile std::uint32_t IFCTL[4];
    };

    // detail::regSet::ClockControl _clkCtrl;
    // detail::regSet::InterruptEventControl _intCtrl;

    // std::array<CcpChannel, 4> _ccpChannels;

    static constexpr uintptr_t clockRegOffset = 0x1000;
    static constexpr uintptr_t commonRegOffset = 0x1100;
    static constexpr uintptr_t ctrRegOffset = 0x1800;

    ClockRegisters* const _clkRegs;
    CommonRegisters* const _commonRegs;
    CounterRegisters* const _ctrRegs;

    // fn_ref _callback;    // TODO implement
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_TIMER_HPP
