#ifndef REGSET_HPP
#define REGSET_HPP

#include <cstdint>
#include <new>

namespace mspm0::detail::regSet {

class PowerControl {
  public:
    constexpr PowerControl(uintptr_t periphAddr) noexcept
     : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
    { }

    constexpr void enable() const noexcept
    {
        _regs->PWREN = (pwrenKey << 24) | 1;    // TODO magic number
    }

    constexpr void reset() const noexcept
    {
        _regs->RSTCTL = (rstEnKey << 24) | 0x11;    // TODO magic numbers
    }

  private:
    struct Registers {
        std::uint32_t PWREN;
        std::uint32_t RSTCTL;
    };

    static constexpr uintptr_t regOffset = 0x800;

    static constexpr std::uint32_t pwrenKey = 0x26;
    static constexpr std::uint32_t rstEnKey = 0xb1;

    volatile Registers* const _regs;
};

class ClockControl {
  public:
    enum class ClockSource : std::uint32_t {
        BusClk = 8,
        MfClk = 4,
        LfClk = 2,
    };

    constexpr ClockControl(uintptr_t periphAddr) noexcept
     : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
    { }

    constexpr void setDivider(unsigned int div) const noexcept
    {
        if (div > 8) {
            return;
        }
        _regs->CKLDIV = div - 1;
    }

    constexpr void setSource(ClockSource src) const noexcept
    {
        _regs->CLKSEL = static_cast<std::underlying_type_t<ClockSource>>(src);
    }

  private:
    struct Registers {
        std::uint32_t CKLDIV;
        std::uint32_t reserved;
        std::uint32_t CLKSEL;
    };

    static constexpr uintptr_t regOffset = 0x1000;

    volatile Registers* const _regs;
};

static constexpr uintptr_t intRegOffset = 0x1020;
static constexpr uintptr_t ev0RegOffset = 0x1050;
static constexpr uintptr_t ev1RegOffset = 0x1080;

class InterruptControl {
  public:
    constexpr InterruptControl(uintptr_t periphAddr, uintptr_t regOffset) noexcept
     : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
    { }

    constexpr void enableInterrupts(std::uint32_t mask) const noexcept { _regs->IMASK |= mask; }
    constexpr void disableInterrupts(std::uint32_t mask) const noexcept { _regs->IMASK &= ~mask; }
    constexpr std::uint32_t getPending() const noexcept { return _regs->IIDX; }

  private:
    struct Registers {
        std::uint32_t IIDX;
        std::uint32_t reserved_0;
        std::uint32_t IMASK;
        std::uint32_t reserved_1;
        std::uint32_t RIS;
        std::uint32_t reserved_2;
        std::uint32_t MIS;
        std::uint32_t reserved_3;
        std::uint32_t ISET;
        std::uint32_t reserved_4;
        std::uint32_t ICLR;
    };

    volatile Registers* const _regs;
};

}    // namespace mspm0::detail::regSet

#endif
