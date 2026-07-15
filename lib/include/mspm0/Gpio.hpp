#ifndef LIB_INCLUDE_MSPM0_GPIO_HPP
#define LIB_INCLUDE_MSPM0_GPIO_HPP

#include "CommonRegs.hpp"

#include <new>
#include <cstdint>

namespace mspm0 {

class Gpio : detail::Peripheral<Gpio> {
  public:
    Gpio(std::uintptr_t addr) noexcept
     : detail::Peripheral<Gpio>(addr)
    //  , _clkCtrl(addr)
    { }

    struct Interrupts {
        enum class InterruptVals : std::uint32_t { };
    };    // TODO make not needed...

    void init() noexcept
    {
        _pwrRegs->resetCtl.reset();
        _pwrRegs->powerEn.enable();

        // _clkCtrl.setSource(detail::regSet::ClockControl::ClockSource::BusClk);
    }

  private:
    // detail::regSet::ClockControl _clkCtrl;
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_GPIO_HPP
