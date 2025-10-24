#ifndef SHIFTLIGHT_HPP
#define SHIFTLIGHT_HPP

#include "Timer.hpp"

static constexpr unsigned int minRpm = 4300;    // TODO better name
static constexpr unsigned int targetRpm = 5700;
static constexpr unsigned int blinkRpm = 6000;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRpm > minRpm);
static_assert(blinkRpm > targetRpm);

using namespace std::literals::chrono_literals;

template <typename SYS_TIM_T, typename LED_T>
class ShiftLight {
  public:
    constexpr ShiftLight(const SYS_TIM_T& sysTim, LED_T& leds) noexcept
     : _blinkTimer(sysTim, 100ms)
     , _leds(leds)
    { }

    constexpr void update(unsigned int rpm) noexcept
    {
        setLeds(rpm);
        _leds.show();
    }

  private:
    static constexpr unsigned int numLeds = LED_T::numLeds;

    static constexpr unsigned int threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int stepSize = (targetRpm - minRpm) / (numLeds - 1);    // TODO correct?
        return minRpm + stepSize * ledNo;                                          // TODO check
    }

    constexpr void setLeds(unsigned int rpm) noexcept
    {
        if (rpm >= blinkRpm) {
            if (!_blinkTimer.isElapsed()) {
                return;
            }

            for (unsigned int i = 0; i < numLeds; ++i) {
                _leds.setLed(i, _blinkState ? 0xff : 0);
            }
            _blinkState = !_blinkState;

            _blinkTimer.reload();
            return;
        }

        unsigned int i = 0;
        for (; i < numLeds; ++i) {
            if (rpm < threshold(i)) {
                for (; i < numLeds; ++i) {
                    _leds.setLed(i, 0);
                }

                return;
            }

            _leds.setLed(i, 0xff);
        }
    }

    bool _blinkState{};
    Timer<SYS_TIM_T> _blinkTimer;
    LED_T& _leds;
};

#endif
