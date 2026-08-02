#ifndef APP_INCLUDE_SHIFTLIGHT_HPP
#define APP_INCLUDE_SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

#include <mp-units/systems/si.h>

constexpr mp_units::Unit auto rpm = mp_units::mag<60> * mp_units::si::hertz;

static constexpr auto minRate = 4300 * rpm;    // TODO better name
static constexpr auto targetRate = 5700 * rpm;
static constexpr auto blinkRate = 6000 * rpm;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRate > minRate);
static_assert(blinkRate > targetRate);

constexpr auto blinkInterval = std::chrono::milliseconds(80);

template <typename LED_T, typename CLOCK_T>
class ShiftLight {
  public:
    constexpr ShiftLight(LED_T& leds, const CLOCK_T& clock) noexcept
     : _blinker(clock)
     , _leds(leds)
    { }

    constexpr void update(mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
    {
        if (_overreving) {
            if (rate < (blinkRate - hysteresis)) {
                _overreving = false;
                setLeds(rate);
            } else {
                _blinker.update(_leds);
            }

        } else {
            if (rate >= blinkRate) {
                _overreving = true;
                _blinker.reset();    // reset blinker so it always starts with the same phase
                _blinker.blink(_leds);

            } else {
                setLeds(rate);
            }
        }

        _leds.show();
    }

  private:
    class Blinker {
      public:
        constexpr Blinker(const CLOCK_T& clock) noexcept
         : _timer(clock, blinkInterval)
        { }

        constexpr void update(LED_T& leds) noexcept
        {
            _timer.poll([this, &leds]() noexcept { blink(leds); });
        }

        constexpr void blink(LED_T& leds) noexcept
        {
            for (unsigned int i = 0; i < numLeds; ++i) {
                leds.setLed(i, _state);
            }
            _state = !_state;
        }

        constexpr void reset() noexcept
        {
            _state = initial;
            _timer.reload();
        }

      private:
        static constexpr bool initial = false;

        PolledTimer<CLOCK_T> _timer;
        bool _state = initial;
    };

    static constexpr unsigned int numLeds = LED_T::numLeds;

    static constexpr mp_units::QuantityOf<mp_units::isq::frequency> auto threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int scaler = 1024;    // to reduce rounding error
        constexpr mp_units::quantity stepSize = (targetRate - minRate) * scaler / (numLeds - 1);
        return minRate + stepSize * ledNo / scaler;
    }

    constexpr void setLeds(mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
    {
        unsigned int i = 0;
        for (; i < numLeds; ++i) {
            if (rate < threshold(i)) {
                break;
            }

            _leds.setLed(i, true);
        }

        for (; i < numLeds; ++i) {
            _leds.setLed(i, false);
        }
    }

    static constexpr auto hysteresis = 50 * rpm;

    bool _overreving{};
    Blinker _blinker;
    LED_T& _leds;
};

#endif    // APP_INCLUDE_SHIFTLIGHT_HPP
