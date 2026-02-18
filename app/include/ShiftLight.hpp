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

    constexpr void setLeds(unsigned int rpm) noexcept
    {
        unsigned int i = 0;
        for (; i < numLeds; ++i) {
            // state 0 means all off (so LED 0 is turned on in state 1) -> compare _prevState to i + 1
            const auto thresh = _prevState == i + 1 ? thresholds[i] - hysteresis : thresholds[i];

            if (rpm < thresh) {
                break;
            }

            _leds.setLed(i, true);
        }

        _prevState = i;

        for (; i < numLeds; ++i) {
            _leds.setLed(i, false);
        }
    }

    static constexpr auto thresholds = [] {
        constexpr unsigned int scaler = 1024;    // to reduce rounding error
        constexpr unsigned int stepSize = (targetRpm - minRpm) * scaler / (numLeds - 1);

        std::array<unsigned int, numLeds> thresholds{};

        for (unsigned int i = 0; i < numLeds; ++i) {
            thresholds[i] = minRpm + stepSize * i / scaler;
        }
        return thresholds;
    }();

    static constexpr unsigned int hysteresis = 50;

    bool _overreving{};
    Blinker _blinker;
    unsigned int _prevState{};
    LED_T& _leds;
};

#endif    // APP_INCLUDE_SHIFTLIGHT_HPP
