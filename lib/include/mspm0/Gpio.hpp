#ifndef LIB_INCLUDE_MSPM0_GPIO_HPP
#define LIB_INCLUDE_MSPM0_GPIO_HPP

#include "CommonRegs.hpp"

#include <new>
#include <cstdint>

namespace mspm0 {

class Gpio {
  public:
    Gpio(std::uintptr_t addr) noexcept
     : _pwrCtrl(addr)
    //  , _clkCtrl(addr)
    { }

    void init() noexcept
    {
        _pwrCtrl.reset();
        _pwrCtrl.enable();

        // _clkCtrl.setSource(detail::regSet::ClockControl::ClockSource::BusClk);
    }

  private:
    detail::regSet::PowerControl _pwrCtrl;
    // detail::regSet::ClockControl _clkCtrl;
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_GPIO_HPP
