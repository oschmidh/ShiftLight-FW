#ifndef DISPLAYWRAPPER_HPP
#define DISPLAYWRAPPER_HPP

#include "Tlc59208f.hpp"

template <typename I2C_T>
class DisplayWrapper {
  public:
    using BrightnessType = std::uint8_t;

    constexpr DisplayWrapper(Tlc59208f<I2C_T>& driver) noexcept
     : _ledDriver(driver)
    { }

    void setBrightness(BrightnessType val) noexcept { _ledDriver.setGlobalBrightness(val); }

  private:
    Tlc59208f<I2C_T>& _ledDriver;
};

#endif    // DISPLAYWRAPPER_HPP
