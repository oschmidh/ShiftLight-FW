#ifndef LIB_INCLUDE_MSPM0_COMMONREGS_HPP
#define LIB_INCLUDE_MSPM0_COMMONREGS_HPP

#include <cstdint>
#include <utility>
#include <new>

namespace mspm0::detail {

namespace commonRegs {

class PowerEn {
  public:
    constexpr void enable() noexcept
    {
        _reg = (pwrenKey << 24) | 1;    // TODO magic number
    }

  private:
    static constexpr std::uint32_t pwrenKey = 0x26;

    volatile std::uint32_t _reg;
};

class ResetCtl {
  public:
    constexpr void reset() noexcept
    {
        _reg = (rstEnKey << 24) | 0x11;    // TODO magic numbers
    }

  private:
    static constexpr std::uint32_t rstEnKey = 0xb1;

    volatile std::uint32_t _reg;
};

class ClockDiv {
  public:
    constexpr void setDivider(unsigned int div) noexcept
    {
        if (div > 8) {
            return;
        }
        _reg = div - 1;
    }

  private:
    volatile std::uint32_t _reg;
};

template <typename CLK_SRC_T>
class ClockSel {
  public:
    using ClockSource = CLK_SRC_T;

    constexpr void setSource(ClockSource src) noexcept { _reg = std::to_underlying(src); }

  private:
    volatile std::uint32_t _reg;
};

class IntControlReg {
  public:
    constexpr void enable(std::uint32_t mask) volatile noexcept { IMASK |= mask; }
    constexpr void disable(std::uint32_t mask) volatile noexcept { IMASK &= ~mask; }

    // returns next pending interrupt, sorted by prio defined in iidx reg (1 = highest prio)
    constexpr std::uint32_t getNextPending() const volatile noexcept { return IIDX; }

  private:
    volatile std::uint32_t IIDX;
    volatile std::uint32_t reserved_0;
    volatile std::uint32_t IMASK;
    volatile std::uint32_t reserved_1;
    volatile std::uint32_t RIS;
    volatile std::uint32_t reserved_2;
    volatile std::uint32_t MIS;
    volatile std::uint32_t reserved_3;
    volatile std::uint32_t ISET;
    volatile std::uint32_t reserved_4;
    volatile std::uint32_t ICLR;
};

// class ClockSel {
//   public:
//     enum class ClockSource : std::uint32_t {
//         BusClk = 1 << 3u,
//         MfClk = 1 << 2u,
//         LfClk = 1 << 1u,
//     };

//     constexpr void setSource(ClockSource src) noexcept { _reg = std::to_underlying(src); }

//   private:
//     volatile std::uint32_t _reg;
// };

}    // namespace commonRegs

namespace regSet {

// class ClockControl {
//   public:
//     using ClockSource = commonRegs::ClockSel::ClockSource;

//     constexpr ClockControl(uintptr_t periphAddr) noexcept
//      : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
//     { }

//     constexpr void setDivider(unsigned int div) noexcept { _regs->div.setDivider(div); }
//     constexpr void setSource(ClockSource src) noexcept { _regs->sel.setSource(src); }

//   private:
//     struct Registers {
//         commonRegs::ClockDiv div;
//         std::uint32_t reserved;
//         commonRegs::ClockSel sel;
//     };

//     static constexpr uintptr_t regOffset = 0x1000;

//     // volatile Registers* const _regs;
//     Registers* const _regs;
// };

// class ClockControl2 {    // TODO hack...
//   public:
//     using ClockSource = commonRegs::ClockSel::ClockSource;

//     constexpr ClockControl2(uintptr_t periphAddr) noexcept
//      : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
//     { }

//     constexpr void setDivider(unsigned int div) noexcept { _regs->div.setDivider(div); }
//     constexpr void setSource(ClockSource src) noexcept { _regs->sel.setSource(src); }

//   private:
//     struct Registers {
//         commonRegs::ClockDiv div;
//         commonRegs::ClockSel sel;
//     };

//     static constexpr uintptr_t regOffset = 0x1000;

//     // volatile Registers* const _regs;
//     Registers* const _regs;
// };

// template <typename INTERRUPT_ENUM_T>
// class InterruptControl {
//   public:
//     constexpr InterruptControl(uintptr_t periphAddr, uintptr_t regOffset) noexcept
//      : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
//     { }

//     template <typename... Ts>
//         requires(std::is_same_v<Ts, INTERRUPT_ENUM_T> && ...)
//     constexpr void enableInterrupts(Ts... interrupts) const noexcept
//     {
//         const std::uint32_t mask = ((1u << std::to_underlying(interrupts)) | ...);
//         _regs->IMASK |= mask;
//     }

//     template <typename... Ts>
//         requires(std::is_same_v<Ts, INTERRUPT_ENUM_T> && ...)
//     constexpr void disableInterrupts(Ts... interrupts) const noexcept
//     {
//         const std::uint32_t mask = ((1u << std::to_underlying(interrupts)) | ...);
//         _regs->IMASK &= ~mask;
//     }

//     // returns next pending interrupt, sorted by prio defined in iidx reg (1 = highest prio)
//     constexpr std::optional<INTERRUPT_ENUM_T> getNextPending() const noexcept
//     {
//         const std::uint32_t nextIidx = _regs->IIDX;
//         if (nextIidx == 0) {
//             return std::nullopt;
//         }

//         return std::make_optional(static_cast<INTERRUPT_ENUM_T>(nextIidx - 1));
//     }

//   private:
//     struct Registers {
//         std::uint32_t IIDX;
//         std::uint32_t reserved_0;
//         std::uint32_t IMASK;
//         std::uint32_t reserved_1;
//         std::uint32_t RIS;
//         std::uint32_t reserved_2;
//         std::uint32_t MIS;
//         std::uint32_t reserved_3;
//         std::uint32_t ISET;
//         std::uint32_t reserved_4;
//         std::uint32_t ICLR;
//     };

//     volatile Registers* const _regs;
// };

// class DebugControl {
//   public:
//     constexpr DebugControl(uintptr_t periphAddr, uintptr_t regOffset) noexcept
//      : _regs(new (reinterpret_cast<std::uint32_t*>(periphAddr + regOffset)) Registers)
//     { }

//     constexpr void enableInterrupts(std::uint32_t mask) const noexcept { _regs->IMASK |= mask; }
//     constexpr void disableInterrupts(std::uint32_t mask) const noexcept { _regs->IMASK &= ~mask; }
//     constexpr std::uint32_t getPending() const noexcept { return _regs->IIDX; }

//   private:
//          IIDX;

//     volatile std::uint32_t* const _pdbgCtl;
// };

}    // namespace regSet

class Peripheral {
  public:
    Peripheral(std::uintptr_t baseAddr) noexcept
     : _pwrRegs(new (reinterpret_cast<std::uint32_t*>(baseAddr + pwrRegOffset)) PowerRegisters)
     , _intEvRegs(new (reinterpret_cast<std::uint32_t*>(baseAddr + intEvRegOffset)) IntEvRegisters)
    { }

  protected:
    struct PowerRegisters {
        commonRegs::PowerEn powerEn;
        commonRegs::ResetCtl resetCtl;
    };

    struct IntEvRegisters {
        commonRegs::IntControlReg intCtrl;
        std::uint32_t reserved_0;
        commonRegs::IntControlReg ev0Ctrl;
        std::uint32_t reserved_1;
        commonRegs::IntControlReg ev1Ctrl;
    };

    PowerRegisters* const _pwrRegs;
    IntEvRegisters* const _intEvRegs;

  private:
    static constexpr uintptr_t pwrRegOffset = 0x800;
    static constexpr uintptr_t intEvRegOffset = 0x1020;
};

}    // namespace mspm0::detail

#endif    // LIB_INCLUDE_MSPM0_COMMONREGS_HPP
