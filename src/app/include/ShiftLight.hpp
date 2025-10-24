#ifndef SHIFTLIGHT_HPP
#define SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

static constexpr unsigned int minRpm = 4300;    // TODO better name
static constexpr unsigned int targetRpm = 5700;
static constexpr unsigned int blinkRpm = 6000;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRpm > minRpm);
static_assert(blinkRpm > targetRpm);

using namespace std::literals::chrono_literals;

template <typename LED_T>
class ShiftLight {
  public:
    constexpr ShiftLight(LED_T& leds) noexcept
     : _blinkTimer(80ms)
     , _leds(leds)
    { }

    constexpr void update(unsigned int rpm) noexcept
    {
        if (rpm >= blinkRpm) {
            _blinkTimer.poll([this]() noexcept { toggleLeds(); });
        } else {
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

    bool _blinkState{};
    PolledTimer _blinkTimer;
    LED_T& _leds;
};

#endif    // SHIFTLIGHT_HPP
