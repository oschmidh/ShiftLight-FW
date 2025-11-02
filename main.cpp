#include "ShiftLight.hpp"
#include "LedBuffer.hpp"
#include "Tlc59208f.hpp"
#include "I2c.hpp"
#include "CaptureTim.hpp"
#include "System.hpp"

#include "ti_msp_dl_config.h"

#include <cstdint>

static constexpr unsigned int numLeds = 8;    // TODO define where?

void startupAnimation(auto& leds) noexcept
{
    using namespace std::literals::chrono_literals;

    for (unsigned int i = 0; i < numLeds; ++i) {
        leds.setLed(i, 0xff);
        leds.show();
        System::busyWait(80ms);
    }

    System::busyWait(1250ms);

    for (int i = numLeds - 1; i >= 0; --i) {
        leds.setLed(i, 0);
        leds.show();
        System::busyWait(80ms);
    }
}

[[noreturn]] int main()
{
    SYSCFG_DL_init();

    System::SteadyClock::init();

    CaptureTimG timG8;    // TODO rename variable?
    timG8.init();

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

    ledDriver.setGlobalBrightness(0x30);

    LedBuffer<Tlc59208f<I2c>, numLeds> leds(ledDriver);
    ShiftLight shiftLight(leds);

    timG8.enable();

    startupAnimation(leds);

    while (1) {

        auto updateLeds = [&shiftLight]<typename REP_T, typename PERIOD_T>(
                              std::chrono::duration<REP_T, PERIOD_T> period) noexcept {
            static constexpr unsigned int scaler = 1 << 20;
            // TODO explain why /2
            const auto rpm =
                scaler /
                std::chrono::duration_cast<std::chrono::duration<unsigned int, std::ratio<60, 2 * scaler>>>(period)
                    .count();

            shiftLight.update(rpm);
        };

        timG8.getPeriod().transform(updateLeds);

        // TODO implement dimming based on ambient light sensor?

        __WFE();
    }
}
