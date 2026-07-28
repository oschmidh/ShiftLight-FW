#ifndef LIB_INCLUDE_MSPM0_DETAIL_COMMONREGS_HPP
#define LIB_INCLUDE_MSPM0_DETAIL_COMMONREGS_HPP

#include <cstdint>
#include <utility>
#include <optional>
#include <new>

namespace mspm0::detail::commonRegs {

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

template <typename INTERRUPT_ENUM_T>
    requires(std::is_same_v<std::underlying_type_t<INTERRUPT_ENUM_T>, std::uint32_t>)
class IntControlReg {
  public:
    template <typename... Ts>
        requires(std::is_same_v<Ts, INTERRUPT_ENUM_T> && ...)
    void enable(Ts... interrupts) noexcept
    {
        const std::uint32_t mask = ((1u << std::to_underlying(interrupts)) | ...);
        IMASK |= mask;
    }

    template <typename... Ts>
        requires(std::is_same_v<Ts, INTERRUPT_ENUM_T> && ...)
    void disable(Ts... interrupts) noexcept
    {
        const std::uint32_t mask = ((1u << std::to_underlying(interrupts)) | ...);
        IMASK &= ~mask;
    }

    // returns next pending interrupt, sorted by prio defined in iidx reg (1 = highest prio)
    std::optional<INTERRUPT_ENUM_T> getNextPending() const noexcept
    {
        const std::uint32_t nextIidx = IIDX;
        if (nextIidx == 0) {
            return std::nullopt;
        }

        return std::make_optional(static_cast<INTERRUPT_ENUM_T>(nextIidx - 1));
    }

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

}    // namespace mspm0::detail::commonRegs

#endif    // LIB_INCLUDE_MSPM0_DETAIL_COMMONREGS_HPP
