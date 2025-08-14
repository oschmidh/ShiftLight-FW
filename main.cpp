#include "I2c.hpp"
#include "Tlc59208f.hpp"

#include "ti_msp_dl_config.h"

#include <array>
#include <cstdint>
#include <optional>

static constexpr unsigned int targetRpm = 5000;
static constexpr unsigned int minRpm = 4000;    // TODO better name

static constexpr unsigned int numLeds = 8;
static constexpr unsigned int colorDepth = 16;                                    // amount of brightness steps used
static constexpr unsigned int stepSize = (targetRpm - minRpm) / (numLeds - 1);    // TODO correct?

constexpr unsigned int threshold(unsigned int ledNo) noexcept
{
    return minRpm + stepSize * ledNo;    // TODO check
}

void updateLeds(auto& leds, unsigned int rpm) noexcept
{    // TODO better name?
    unsigned int i = 0;
    for (; i < numLeds; ++i) {
        if (rpm < threshold(i)) {
            break;
        }

        leds.setLed(i, (rpm - threshold(i)) * colorDepth / stepSize);
    }

    for (; i < numLeds; ++i) {
        leds.setLed(i, 0);
    }

    // TODO implement blinking at overreving?
}

template <typename DRIVER_T>
class LedController {
  public:
    constexpr LedController(DRIVER_T& ledDriver) noexcept
     : _driver(ledDriver)
    { }

    constexpr void init() const noexcept
    {
        [this]<unsigned int... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
            _driver.configureChannels(
                (Tlc59208f<I2c>::ChannelConfig{IDX_Vs, Tlc59208f<I2c>::DriverState::GroupCtrl}, ...));
        }(std::make_index_sequence<numLeds>());
    }

    constexpr void setLed(unsigned int idx, std::uint8_t brightness) noexcept
    {
        if (idx >= numLeds) {
            return;
        }

        if (_ledBuf[idx] == brightness) {
            return;
        }

        _ledBuf[idx] = brightness;
    }

    constexpr void refresh() noexcept
    {
        for (unsigned int i = 0; i < _ledBuf.size(); ++i) {
            if (!_ledBuf[i].has_value()) {
                continue;
            }

            _driver.setBrightness(i, _ledBuf[i].value());
            _ledBuf[i].reset();
        }
    }

  private:
    std::array<std::optional<std::uint8_t>, numLeds> _ledBuf{};
    const DRIVER_T& _driver;
};

[[noreturn]] int main()
{
    SYSCFG_DL_init();

    I2c i2c0;
    i2c0.init();

    static constexpr std::uint8_t ledDriverI2cAddr = 0xff;    // TODO define somewhere else
    Tlc59208f driver(i2c0, ledDriverI2cAddr);

    LedController leds(driver);
    leds.init();

    // TODO add and configure timer

    // TODO display startup animation

    while (1) {
        const unsigned int rpm = 4300;    // TODO fixed for test

        updateLeds(leds, rpm);

        // TODO implement dimming based on ambient light sensor?

        // TODO delay?
    }
}
