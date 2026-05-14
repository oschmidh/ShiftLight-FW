#ifndef LIB_INCLUDE_MSPM0_BASETIMER_HPP
#define LIB_INCLUDE_MSPM0_BASETIMER_HPP

#include "RegSet.hpp"

#include <utility>
#include <cstdint>

namespace mspm0 {

class BaseTimer {
  public:
    class CcpChannel {
      public:
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

        constexpr CcpChannel(uintptr_t addr, unsigned int channelId) noexcept    // TODO ensure channelId < 4
         : CC(new (reinterpret_cast<std::uint32_t*>(addr + ccOffset + channelId * sizeof(std::uint32_t))) std::uint32_t)
         , CCCTL(new (reinterpret_cast<std::uint32_t*>(addr + ccctlOffset + channelId * sizeof(std::uint32_t)))
                     std::uint32_t)
         , OCTL(new (reinterpret_cast<std::uint32_t*>(addr + octlOffset + channelId * sizeof(std::uint32_t)))
                    std::uint32_t)
         , CCACT(new (reinterpret_cast<std::uint32_t*>(addr + ccactOffset + channelId * sizeof(std::uint32_t)))
                     std::uint32_t)
         , IFCTL(new (reinterpret_cast<std::uint32_t*>(addr + ifctlOffset + channelId * sizeof(std::uint32_t)))
                     std::uint32_t)
        { }

        void configure(const CaptureCompareConfig& cfg) noexcept
        {
            *CCCTL = std::to_underlying(cfg.captureOrCompare) | (std::to_underlying(cfg.captureOrCompare) << 17u) |
                     (std::to_underlying(cfg.zeroCondition) << 12u) | (std::to_underlying(cfg.loadCondition) << 8u) |
                     (std::to_underlying(cfg.advanceCondition) << 4u) |
                     (std::to_underlying(cfg.captureCondition));    // TODO
                                                                    // magic
                                                                    // offsets
        }

        std::uint32_t getValue() const noexcept { return *CC; }

        void setValue(std::uint32_t val) noexcept { *CC = val; }

      private:
        static constexpr uintptr_t ccOffset = 0x1810;
        volatile std::uint32_t* CC;

        static constexpr uintptr_t ccctlOffset = 0x1830;
        volatile std::uint32_t* CCCTL;

        static constexpr uintptr_t octlOffset = 0x1850;
        volatile std::uint32_t* OCTL;

        static constexpr uintptr_t ccactOffset = 0x1870;
        volatile std::uint32_t* CCACT;

        static constexpr uintptr_t ifctlOffset = 0x1880;
        volatile std::uint32_t* IFCTL;
    };

    enum class CcpDirection { Input = 0, Output = 1 };

    constexpr BaseTimer(uintptr_t addr) noexcept
     : _pwrCtrl(addr)
     , _clkCtrl(addr)
     , _intCtrl(addr, detail::regSet::intRegOffset)
     , _ccpChannels{{{addr, 0u}, {addr, 1u}, {addr, 2u}, {addr, 3u}}}
     , _commonRegs(new (reinterpret_cast<std::uint32_t*>(addr + commonRegOffset)) CommonRegisters)
     , _ctrRegs(new (reinterpret_cast<std::uint32_t*>(addr + ctrRegOffset)) CounterRegisters)
    { }

    void init(unsigned int prescaler) noexcept
    {
        _pwrCtrl.reset();
        _pwrCtrl.enable();

        _clkCtrl.setSource(detail::regSet::ClockControl::ClockSource::BusClk);
        _commonRegs->CPS = prescaler;
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

    void configure(const Config& cfg) noexcept
    {
        _ctrRegs->CTRCTL = (std::to_underlying(cfg.ctrValAfterEn) << 28u) | (cfg.phaseLoadEn << 24u) |
                           (cfg.ctrZeroControl << 13u) | (cfg.ctrAdvanceControl << 10u) | (cfg.ctrLoadControl << 7u) |
                           (std::to_underlying(cfg.countMode) << 4u) |
                           (std::to_underlying(cfg.repeat) << 1u);    // TODO magic
                                                                      // offsets
    }

    void configureCcpDirection(unsigned int channel, CcpDirection dir) noexcept
    {
        // TODO verify channel in range?
        _commonRegs->CCPD &= ~(1u << channel);
        _commonRegs->CCPD |= std::to_underlying(dir) << channel;
    }

    const CcpChannel& getCcpChannel(unsigned int channel) const noexcept { return _ccpChannels[channel]; }
    CcpChannel& getCcpChannel(unsigned int channel) noexcept { return _ccpChannels[channel]; }

    void setCounter(std::uint32_t val) noexcept { _ctrRegs->CTR = val; }

    void enableClock() noexcept { _commonRegs->CCLKCTL |= 1; }

    void disableClock() noexcept { _commonRegs->CCLKCTL &= ~1; }

    void enableInterrupts(std::uint32_t mask) noexcept { _intCtrl.enableInterrupts(mask); }

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
    };

    detail::regSet::PowerControl _pwrCtrl;
    detail::regSet::ClockControl _clkCtrl;
    detail::regSet::InterruptControl _intCtrl;

    std::array<CcpChannel, 4> _ccpChannels;

    static constexpr uintptr_t commonRegOffset = 0x1100;
    static constexpr uintptr_t ctrRegOffset = 0x1800;

    volatile CommonRegisters* const _commonRegs;
    volatile CounterRegisters* const _ctrRegs;

    // fn_ref _callback;    // TODO implement
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_BASETIMER_HPP
