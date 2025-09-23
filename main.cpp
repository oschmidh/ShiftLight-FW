#include "ShiftLight.hpp"
#include "LedBuffer.hpp"
#include "Tlc59208f.hpp"
#include "I2c.hpp"
#include "CaptureTim.hpp"

#include "ti_msp_dl_config.h"

#include <cstdint>

static constexpr unsigned int numLeds = 8;    // TODO define where?

[[noreturn]] int main()
{
    SYSCFG_DL_init();

    CaptureTimA timA0;    // TODO rename variable?
    timA0.init();

    I2c i2c0;
    i2c0.init();

    static constexpr std::uint8_t ledDriverI2cAddr = 0x20;    // TODO define somewhere else
    Tlc59208f ledDriver(i2c0, ledDriverI2cAddr);
    ledDriver.configure({.mode = Tlc59208f<I2c>::Mode::Normal});    // TODO get rid of template param in enum

    // TODO kinda ugly api...
    ledDriver.configureChannels(
        Tlc59208f<I2c>::ChannelConfig{.channel = 0, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 1, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 2, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 3, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 4, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 5, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 6, .state = Tlc59208f<I2c>::DriverState::GroupCtrl},
        Tlc59208f<I2c>::ChannelConfig{.channel = 7, .state = Tlc59208f<I2c>::DriverState::GroupCtrl});

    LedBuffer<Tlc59208f<I2c>, numLeds> leds(ledDriver);
    ShiftLight shiftLight(leds);

    timA0.enable();

    // TODO display startup animation

    while (1) {

        static constexpr unsigned int scaler = 1 << 20;
        // TODO explain why /2
        const auto rpm =
            scaler / std::chrono::duration_cast<std::chrono::duration<unsigned int, std::ratio<60, 2 * scaler>>>(
                         timA0.getPeriod())
                         .count();

        shiftLight.update(rpm);

        // TODO implement dimming based on ambient light sensor?

        // TODO delay?
    }
}
