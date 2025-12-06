#ifndef APP_INCLUDE_SHIFTLIGHT_HPP
#define APP_INCLUDE_SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

using namespace std::literals::chrono_literals;

struct ShiftLightConfig {
    unsigned int minRpm;    // TODO better name
    unsigned int targetRpm;
    unsigned int blinkRpm;   // TODO auto derive from targetRpm + stepsize
};

template <ShiftLightConfig CFG_V, typename LED_T>
class ShiftLight {
  public:
    static_assert(CFG_V.targetRpm > CFG_V.minRpm);
    static_assert(CFG_V.blinkRpm > CFG_V.targetRpm);

    constexpr ShiftLight(LED_T& leds) noexcept
     : _blinkTimer(80ms)
     , _leds(leds)
    { }

    constexpr void update(unsigned int rpm) noexcept
    {
        if (rpm >= _overrevTh) {
            _overrevTh = CFG_V.blinkRpm - hysteresis;
            _blinkTimer.poll([this]() noexcept { toggleLeds(); });
        } else {
            _overrevTh = CFG_V.blinkRpm;
            setLeds(rpm);
        }

        _leds.show();
    }

  private:
    static constexpr unsigned int numLeds = LED_T::numLeds;

    static constexpr unsigned int threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int stepSize = (CFG_V.targetRpm - CFG_V.minRpm) / (numLeds - 1);    // TODO correct?
        return CFG_V.minRpm + stepSize * ledNo;                                          // TODO check
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

    unsigned int _overrevTh{CFG_V.blinkRpm};
    bool _blinkState{};
    PolledTimer _blinkTimer;
    LED_T& _leds;
};

#endif // APP_INCLUDE_SHIFTLIGHT_HPP
