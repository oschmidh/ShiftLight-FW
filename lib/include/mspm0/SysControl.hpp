#ifndef LIB_INCLUDE_MSPM0_SYSCONTROL_HPP
#define LIB_INCLUDE_MSPM0_SYSCONTROL_HPP

#include <new>
#include <utility>
#include <cstdint>

namespace mspm0 {

class SysControl {
  public:
    enum class SysOscFreq {
        Base32Mhz = 0,
        Low4Mhz = 1,
        UserTrimmed = 2,
    };

    struct SysOscCfg {
        bool fastCpuEvent = true;
        bool blockAsyncAll = false;
        bool disable = false;
        bool disableInStop = false;
        SysOscFreq freq = SysOscFreq::Base32Mhz;
    };

    struct MclkCfg {
        bool useLfclk = false;
        bool useHsclk = false;
        bool useMfclk = false;
        unsigned int divider = 0;    // TODO ensure 4 byte?
    };

    constexpr SysControl(uintptr_t addr) noexcept
     : _regs(new (reinterpret_cast<std::uint32_t*>(addr + regOffset)) Registers)
    { }

    void configureSysOsc(const SysOscCfg& cfg) const noexcept
    {
        _regs->SysOscCfg = (cfg.fastCpuEvent << 17u) | (cfg.blockAsyncAll << 16u) | (cfg.disable << 10u) |
                           (cfg.disableInStop << 9u) | std::to_underlying(cfg.freq);
    }

    void configureMclk(const MclkCfg& cfg) const noexcept
    {
        _regs->MclkCfg = (cfg.useLfclk << 20u) | (cfg.useHsclk << 16u) | (cfg.useMfclk << 12u) | cfg.divider;
    }

    void disableNrstPin() const noexcept
    {
        static constexpr std::uint32_t key = 0x1e;
        _regs->ExRstPin = (key << 24u) | 1;
    }

  private:
    struct Registers {    // TODO
        std::uint32_t SysOscCfg;
        std::uint32_t MclkCfg;
        std::uint32_t placeholder_0[1];    // TODO fix size
        // std::uint32_t SystemCfg;
        // std::uint32_t BeepCfg;
        // std::uint32_t WriteLock;
        // std::uint32_t ClkStatus;
        // std::uint32_t SysStatus;
        // std::uint32_t RstCause;
        // std::uint32_t ResetLevel;
        // std::uint32_t ResetCmd;
        // std::uint32_t BorThreshold;
        // std::uint32_t BorClrCmd;
        // std::uint32_t SysOscFclCtl;
        // std::uint32_t ExLfctl;
        // std::uint32_t ShdnIoRel;
        std::uint32_t ExRstPin;
        // std::uint32_t SysStatusClr;
        // std::uint32_t SwdCfg;
        // std::uint32_t FccCmd;

        // std::uint32_t reserved_0;
        // std::uint32_t IMASK;
        // std::uint32_t reserved_1;
        // std::uint32_t RIS;
        // std::uint32_t reserved_2;
        // std::uint32_t MIS;
        // std::uint32_t reserved_3;
        // std::uint32_t ISET;
        // std::uint32_t reserved_4;
        // std::uint32_t ICLR;
    };

    static constexpr uintptr_t regOffset = 0x800;    // TODO

    volatile Registers* const _regs;
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_SYSCONTROL_HPP
