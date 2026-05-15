#include "ShiftLight.hpp"
#include "LedBuffer.hpp"
#include "Devices.hpp"
#include "System.hpp"
#include <drivers/Tlc59208f.hpp>

#include "ti_msp_dl_config.h"

#include <cstdint>

static constexpr unsigned int numLeds = 8;    // TODO define where?

void startupAnimation(auto& clock, auto& leds) noexcept
{
    using namespace std::literals::chrono_literals;

    for (unsigned int i = 0; i < numLeds; ++i) {
        leds.setLed(i, true);
        leds.show();
        busyWait(clock, 80ms);
    }

    busyWait(clock, 1250ms);

    for (int i = numLeds - 1; i >= 0; --i) {
        leds.setLed(i, false);
        leds.show();
        busyWait(clock, 80ms);
    }
}

[[noreturn]] int main()
{
    Devices::gpioA.init();

    delay_cycles(POWER_STARTUP_DELAY);    // TODO ??

    Devices::sysCtl.disableNrstPin();

    Devices::pin1->configure(
        {.function = mspm0::IoMux::Pin1Functions::I2c0_Sda, .connected = true, .inputEnable = true, .openDrain = true});
    Devices::pin2->configure(
        {.function = mspm0::IoMux::Pin2Functions::I2c0_Scl, .connected = true, .inputEnable = true, .openDrain = true});
    Devices::pin28->configure(
        {.function = mspm0::IoMux::Pin28Functions::TimG8_Ccp1, .connected = true, .inputEnable = true});

    Devices::sysCtl.configureSysOsc({.freq = mspm0::SysControl::SysOscFreq::Base32Mhz});
    Devices::sysCtl.configureMclk({.divider = 0});

    System::SteadyClock sysTime{};
    sysTime.init();

    Devices::timG8.init();

    Devices::i2c0.init();

    static constexpr std::uint8_t ledDriverI2cAddr = 0x20;    // TODO define somewhere else
    Tlc59208f ledDriver(Devices::i2c0, ledDriverI2cAddr);
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

    // correction values to account for the different brightness values and human perception
    // clang-format off
    static constexpr std::array<std::uint8_t, numLeds> brightnessTable = {
                                                                           0xff, 0xff, 0xff, // green
                                                                           0x26, 0x26, 0x26, // yellow
                                                                           0x2e, 0x2e        // red
                                                                         };
    // clang-format on
    LedBuffer<Tlc59208f<I2c>, numLeds, brightnessTable> leds(ledDriver);
    ShiftLight shiftLight(leds, sysTime);

    Devices::timG8.enable();

    startupAnimation(sysTime, leds);

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

        Devices::timG8.getPeriod().transform(updateLeds);

        // TODO implement dimming based on ambient light sensor?

        __WFE();
    }
}
