#include "ShiftLight.hpp"
#include "LedBuffer.hpp"
#include "Devices.hpp"
#include "System.hpp"
#include <drivers/Tlc59208f.hpp>

#include "ti_msp_dl_config.h"

#include <variant>
#include <cstdint>

static constexpr unsigned int numLeds = 8;    // TODO define where?

void startupAnimation(auto& leds) noexcept
{
    using namespace std::literals::chrono_literals;

    for (unsigned int i = 0; i < numLeds; ++i) {
        leds.setLed(i, true);
        leds.show();
        System::busyWait(80ms);
    }

    System::busyWait(1250ms);

    for (int i = numLeds - 1; i >= 0; --i) {
        leds.setLed(i, false);
        leds.show();
        System::busyWait(80ms);
    }

    System::busyWait(1250ms);

    for (unsigned int i = 0; i < numLeds; ++i) {
        leds.setLed(i, true);
        leds.show();
        System::busyWait(80ms);
    }

    System::busyWait(1250ms);

    for (int i = numLeds - 1; i >= 0; --i) {
        leds.setLed(i, false);
        leds.show();
        System::busyWait(80ms);
    }
}

namespace Event {

struct TimerCapture {
    CaptureTimG::PeriodType period;
};

// struct CaptureTimeout { };

}    // namespace Event

// using Events = std::variant<Event::TimerCapture, Event::CaptureTimeout>;
using Events = std::variant<Event::TimerCapture>;

// TODO locking policy

// template <typename T, std::size_t N>
// // TODO equires N = power of 2 and N < 2^sizeof(std::size_t)
// class Ringbuffer {
//   public:
//     constexpr bool isEmpty() volatile const noexcept
//     {
//         // TODO lock interrupts here
//         return _tail == _head;
//     }

//     constexpr bool isFull() volatile const noexcept
//     {
//         // TODO lock interrupts here
//         return size() == _buf.size();
//     }

//     constexpr std::size_t size() volatile const noexcept { return _head - _tail; }

//     constexpr bool push(T&& b) volatile noexcept
//     {
//         if (isFull()) {
//             return false;
//         }
//         _buf[mask(_head++)] = b;
//         return true;
//     }

//     constexpr std::optional<T> pull() volatile noexcept
//     {
//         if (isEmpty()) {
//             return std::nullopt;
//         }
//         return _buf[mask(_tail++)];
//     }

//   private:
//     static constexpr std::size_t mask(std::size_t val) noexcept { return val & (N - 1); }

//     std::size_t _tail{};
//     std::size_t _head{};
//     std::array<T, N> _buf;
// };

template <typename T, std::size_t N>
// TODO equires N = power of 2 and N < 2^sizeof(std::size_t)
class Ringbuffer {
  public:
    constexpr bool isEmpty() const noexcept
    {
        // TODO lock interrupts here
        return _tail == _head;
    }

    constexpr bool isFull() const noexcept
    {
        // TODO lock interrupts here
        return size() == _buf.size();
    }

    constexpr std::size_t size() const noexcept { return _head - _tail; }

    constexpr bool push(T&& b) noexcept
    {
        if (isFull()) {
            return false;
        }
        _buf[mask(_head++)] = b;
        return true;
    }

    constexpr std::optional<T> pull() noexcept
    {
        if (isEmpty()) {
            return std::nullopt;
        }
        return _buf[mask(_tail++)];
    }

  private:
    static constexpr std::size_t mask(std::size_t val) noexcept { return val & (N - 1); }

    std::size_t _tail{};
    std::size_t _head{};
    std::array<T, N> _buf;
};

// volatile Ringbuffer<Events, 8> eventBuf;
Ringbuffer<Events, 8> eventBuf;

namespace {

void captureIsr(CaptureTimG::PeriodType period) noexcept { eventBuf.push(Event::TimerCapture{period}); }

}    // namespace

I2c Devices::i2c0;
CaptureTimG Devices::timG8(captureIsr);
TimA0Clock Devices::timA0;

[[noreturn]] int main()
{
    SYSCFG_DL_init();

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

    startupAnimation(leds);

    while (1) {

        auto onEvent = [&shiftLight](const Event::TimerCapture& e) noexcept {
            static constexpr unsigned int scaler = 1 << 20;

            const auto rpm =
                scaler /
                std::chrono::duration_cast<std::chrono::duration<unsigned int, std::ratio<60, 2 * scaler>>>(e.period)
                    .count();

            shiftLight.update(rpm);
        };

        // auto onEvent = [&shiftLight](const Event::CaptureTimeout& e) noexcept { shiftLight.update(0); };

        // Devices::timG8.getPeriod().transform(updateLeds);

        // TODO implement dimming based on ambient light sensor?

        // while (!eventBuf.isEmpty()) {    // TODO check not needed, because pull will return nullopt in that case
        //     const auto event = eventBuf.pull();
        //     if (!event.has_value()) {
        //         break;
        //     }
        //     std::visit([onEvent](auto&& ev) { onEvent(ev); }, event.value());
        // }

        const auto event = eventBuf.pull();
        if (event.has_value()) {
            // break;
            std::visit([onEvent](auto&& ev) { onEvent(ev); }, event.value());
        }

        // while (const auto event = eventBuf.pull(); event.has_value()) {
        //     std::visit([](auto&& ev) { onEvent(ev); }, event.value());
        // }

        if (!eventBuf.isEmpty()) {
            continue;
        }

        // if (eventBuf.isEmpty()){
        __WFE();
        // }
    }
}
