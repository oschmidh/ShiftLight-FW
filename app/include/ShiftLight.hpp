#ifndef APP_INCLUDE_SHIFTLIGHT_HPP
#define APP_INCLUDE_SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

static constexpr unsigned int minRpm = 4300;    // TODO better name
static constexpr unsigned int targetRpm = 5700;
static constexpr unsigned int blinkRpm = 6000;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRpm > minRpm);
static_assert(blinkRpm > targetRpm);

using namespace std::literals::chrono_literals;

template <typename LED_T, typename CLOCK_T>
class ShiftLight {
  public:
    constexpr ShiftLight(LED_T& leds, const CLOCK_T& clock) noexcept
     : _blinkTimer(clock, 80ms)
     , _leds(leds)
    { }

    constexpr void update(unsigned int rpm) noexcept
    {
        if (rpm >= _overrevTh) {
            _overrevTh = blinkRpm - hysteresis;
            _blinkTimer.poll([this]() noexcept { toggleLeds(); });
        } else {
            _overrevTh = blinkRpm;
            setLeds(rpm);
        }

        _leds.show();
    }

  private:
    static constexpr unsigned int numLeds = LED_T::numLeds;

    static constexpr unsigned int threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int stepSize = (targetRpm - minRpm) / (numLeds - 1);    // TODO correct?
        return minRpm + stepSize * ledNo;                                          // TODO check
    }

    constexpr void toggleLeds() noexcept
    {
        for (unsigned int i = 0; i < numLeds; ++i) {
            _leds.setLed(i, _blinkState );
        }
        _blinkState = !_blinkState;
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

    unsigned int _overrevTh{blinkRpm};
    bool _blinkState{};
    PolledTimer<CLOCK_T> _blinkTimer;
    LED_T& _leds;
};

#endif // APP_INCLUDE_SHIFTLIGHT_HPP
