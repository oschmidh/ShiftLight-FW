#ifndef LIB_INCLUDE_MSPM0_GPIO_HPP
#define LIB_INCLUDE_MSPM0_GPIO_HPP

#include "detail/Peripheral.hpp"
#include "detail/CommonRegs.hpp"

#include <new>
#include <cstdint>

namespace mspm0 {

class Gpio : detail::Peripheral<Gpio> {
  public:
    Gpio(std::uintptr_t addr, cortex_m0plus::Nvic& nvic) noexcept
     : detail::Peripheral<Gpio>(addr, nvic)
    { }

    struct Interrupts {
        enum class InterruptVals : std::uint32_t { };
    };    // TODO make not needed...

    void init() noexcept
    {
        _pwrRegs->resetCtl.reset();
        _pwrRegs->powerEn.enable();
    }

  private:
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_GPIO_HPP
