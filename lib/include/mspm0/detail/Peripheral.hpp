#ifndef LIB_INCLUDE_MSPM0_DETAIL_PERIPHERAL_HPP
#define LIB_INCLUDE_MSPM0_DETAIL_PERIPHERAL_HPP

#include "CommonRegs.hpp"

#include <cortex_m0plus/Nvic.hpp>

#include <cstdint>
#include <utility>
#include <optional>
#include <new>

namespace mspm0::detail {

template <typename DERIVED_T>
class Peripheral {
  public:
    Peripheral(std::uintptr_t baseAddr, cortex_m0plus::Nvic& nvic) noexcept
     : _pwrRegs(new (reinterpret_cast<std::uint32_t*>(baseAddr + pwrRegOffset)) PowerRegisters)
     , _intEvRegs(new (reinterpret_cast<std::uint32_t*>(baseAddr + intEvRegOffset)) IntEvRegisters)
     , _nvic(nvic)
    { }

    void enableInterruptLine(unsigned int idx) noexcept { _nvic.enableInterrupt(idx); }
    void disableInterruptLine(unsigned int idx) noexcept { _nvic.disableInterrupt(idx); }

  protected:
    struct PowerRegisters {
        commonRegs::PowerEn powerEn;
        commonRegs::ResetCtl resetCtl;
    };

    struct IntEvRegisters {
        commonRegs::IntControlReg<typename DERIVED_T::Interrupts::InterruptVals> intCtrl;
        std::uint32_t reserved_0;
        commonRegs::IntControlReg<typename DERIVED_T::Interrupts::InterruptVals> ev0Ctrl;
        std::uint32_t reserved_1;
        commonRegs::IntControlReg<typename DERIVED_T::Interrupts::InterruptVals> ev1Ctrl;
    };

    PowerRegisters* const _pwrRegs;
    IntEvRegisters* const _intEvRegs;

  private:
    static constexpr uintptr_t pwrRegOffset = 0x800;
    static constexpr uintptr_t intEvRegOffset = 0x1020;

    cortex_m0plus::Nvic& _nvic;
};

}    // namespace mspm0::detail

#endif    // LIB_INCLUDE_MSPM0_DETAIL_PERIPHERAL_HPP
