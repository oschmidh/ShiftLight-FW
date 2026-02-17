#ifndef APP_INCLUDE_SHIFTLIGHT_HPP
#define APP_INCLUDE_SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

static constexpr unsigned int minRpm = 4300;    // TODO better name
static constexpr unsigned int targetRpm = 5700;
static constexpr unsigned int blinkRpm = 6000;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRpm > minRpm);
static_assert(blinkRpm > targetRpm);

constexpr auto blinkInterval = std::chrono::milliseconds(80);

template <typename LED_T, typename CLOCK_T>
class ShiftLight {
  public:
    constexpr ShiftLight(LED_T& leds, const CLOCK_T& clock) noexcept
     : _blinker(clock)
     , _leds(leds)
    { }

    constexpr void update(unsigned int rpm) noexcept
    {
        if (_overreving) {
            if (rpm < (blinkRpm - hysteresis)) {
                _overreving = false;
                setLeds(rpm);
            } else {
                _blinker.update(_leds);
            }

        } else {
            if (rpm >= blinkRpm) {
                _overreving = true;
                _blinker.reset();    // reset blinker so it always starts with the same phase
                _blinker.blink(_leds);

            } else {
                setLeds(rpm);
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

    static constexpr unsigned int threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int scaler = 1024;    // to reduce rounding error
        constexpr unsigned int stepSize = (targetRpm - minRpm) * scaler / (numLeds - 1);
        return minRpm + stepSize * ledNo / scaler;
    }

    constexpr void setLeds(unsigned int rpm) noexcept
    {
        unsigned int i = 0;
        for (; i < numLeds; ++i) {
            if (rpm < threshold(i)) {
                break;
            }

            _leds.setLed(i, true);
        }

        for (; i < numLeds; ++i) {
            _leds.setLed(i, false);
        }
    }

    static constexpr unsigned int hysteresis = 50;

    bool _overreving{};
    Blinker _blinker;
    LED_T& _leds;
};

#endif // APP_INCLUDE_SHIFTLIGHT_HPP
