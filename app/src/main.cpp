#include "ShiftLight.hpp"
#include "AdaptiveDimming.hpp"
#include "LedBuffer.hpp"
#include "Devices.hpp"
#include "System.hpp"
#include "MpUnitsChronoConv.hpp"
#include <drivers/Tlc59208f.hpp>
#include "Veml7700.hpp"

#include <mp-units/systems/si.h>

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
    mspm0::peripherals::gpio0.init();

    mspm0::peripherals::sysCtl.disableNrstPin();

    mspm0::peripherals::pin1.configure({.function = mspm0::ioMux::Pin1Functions::I2c0_Sda,
                                        .connected = true,
                                        .inputEnable = true,
                                        .hysteresis = false,
                                        .openDrain = true});
    mspm0::peripherals::pin2.configure({.function = mspm0::ioMux::Pin2Functions::I2c0_Scl,
                                        .connected = true,
                                        .inputEnable = true,
                                        .hysteresis = false,
                                        .openDrain = true});
    mspm0::peripherals::pin28.configure(
        {.function = mspm0::ioMux::Pin28Functions::TimG8_Ccp1, .connected = true, .inputEnable = true});

    mspm0::peripherals::sysCtl.configureSysOsc({.freq = mspm0::SysControl::SysOscFreq::Base32Mhz});
    mspm0::peripherals::sysCtl.configureMclk({.divider = 0});

    System::SteadyClock sysTime{};
    sysTime.init();

    Devices::rpmCaptureTim.init();

    Devices::i2c.init();

    static constexpr std::uint8_t alsI2cAddr = 0x10;    // TODO define somewhere else
    Veml7700 ambientLightSens(i2c0, alsI2cAddr);
    ambientLightSens.configure({.gain = Veml7700<I2c>::Gain::Gain2,
                                .it = Veml7700<I2c>::IntegrationTime::Int100ms,
                                .pers = Veml7700<I2c>::Persistence::Pers4,
                                .interruptEn = false,
                                .shutdown = false});

    static constexpr std::uint8_t ledDriverI2cAddr = 0x20;    // TODO define somewhere else
    Tlc59208f ledDriver(Devices::i2c, ledDriverI2cAddr);
    ledDriver.configure({.mode = Tlc59208f<mspm0::I2cController>::Mode::Normal});    // TODO get rid of template param
                                                                                     // in enum

    constexpr std::array channelCfgs = []() {
        std::array<Tlc59208f<mspm0::I2cController>::ChannelConfig, numLeds> cfgs;

        for (unsigned int i = 0; i < numLeds; ++i) {
            cfgs[i].channel = i;
            cfgs[i].state = Tlc59208f<mspm0::I2cController>::DriverState::GroupCtrl;
        }

        return cfgs;
    }();

    ledDriver.configureChannels({channelCfgs});

    ledDriver.setGlobalBrightness(0x30);

    // correction values to account for the different brightness values and human perception
    // clang-format off
    static constexpr std::array<std::uint8_t, numLeds> brightnessTable = {
                                                                           0xff, 0xff, 0xff, // green
                                                                           0x26, 0x26, 0x26, // yellow
                                                                           0x2e, 0x2e        // red
                                                                         };
    // clang-format on
    LedBuffer<Tlc59208f<mspm0::I2cController>, numLeds, brightnessTable> leds(ledDriver);
    ShiftLight shiftLight(leds, sysTime);

    AdaptiveDimming dimmingControl(ambientLightSens, ledDriver);
    dimmingControl.init();

    Devices::rpmCaptureTim.enable();

    startupAnimation(sysTime, leds);

    while (1) {

        auto updateLeds = [&shiftLight]<typename REP_T, typename PERIOD_T>(
                              std::chrono::duration<REP_T, PERIOD_T> timerPeriod) noexcept {
            static constexpr int pulsesPerCrankshaftRot = 2;
            const mp_units::quantity crankPeriod = timerPeriod * pulsesPerCrankshaftRot;
            shiftLight.update(crankPeriod);
        };

        Devices::rpmCaptureTim.getPeriod().transform(updateLeds);

        dimmingControl.run();

        asm volatile("wfe" ::: "memory");
    }
}
