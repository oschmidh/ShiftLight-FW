#include "I2c.hpp"
#include "CaptureTim.hpp"
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
        ledDriver.configure({.mode = Tlc59208f<I2c>::Mode::Normal});    // TODO get rid of template param in enum

        // TODO kinda ugly api...
        _driver.configureChannels(
            Tlc59208f<I2c>::ChannelConfig{.channel = 0, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 1, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 2, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 3, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 4, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 5, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 6, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
            Tlc59208f<I2c>::ChannelConfig{.channel = 7, .state = Tlc59208f<I2c>::DriverState::GroupCtrl});
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

    CaptureTimA timA0;    // TODO rename variable?
    timA0.init();

    I2c i2c0;
    i2c0.init();

    static constexpr std::uint8_t ledDriverI2cAddr = 0x20;    // TODO define somewhere else
    Tlc59208f ledDriver(i2c0, ledDriverI2cAddr);

    LedController leds(ledDriver);
    leds.init();

    timA0.enable();

    // TODO display startup animation

    while (1) {

        static constexpr unsigned int scaler = 1 << 20;
        // TODO explain why /2
        const auto rpm =
            scaler / std::chrono::duration_cast<std::chrono::duration<unsigned int, std::ratio<60, 2 * scaler>>>(
                         timA0.getPeriod())
                         .count();

        updateLeds(leds, rpm);

        // TODO implement dimming based on ambient light sensor?

        // TODO delay?
    }
}
