#ifndef APP_INCLUDE_SHIFTLIGHT_HPP
#define APP_INCLUDE_SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

using namespace std::literals::chrono_literals;

template <unsigned int MIN_RPM_V, unsigned int TARGET_RPM_V, unsigned int BLINK_RPM_V>
struct ShiftLightCfg {
    static_assert(TARGET_RPM_V > MIN_RPM_V);
    static_assert(BLINK_RPM_V > TARGET_RPM_V);
};

struct ShiftLightCfgParams {
    unsigned int minRpm;    // TODO better name
    unsigned int targetRpm;
    unsigned int blinkRpm;    // TODO auto derive from targetRpm + stepsize
};

template <ShiftLightCfgParams CFG_V>
static constexpr auto makeShiftLightConfig() noexcept -> ShiftLightCfg<CFG_V.minRpm, CFG_V.targetRpm, CFG_V.blinkRpm>
{
    return {};
}

template <typename LED_T, typename CONFIG_T>
class ShiftLight;

template <typename LED_T, unsigned int MIN_RPM_V, unsigned int TARGET_RPM_V, unsigned int BLINK_RPM_V>
class ShiftLight<LED_T, ShiftLightCfg<MIN_RPM_V, TARGET_RPM_V, BLINK_RPM_V>> {
  public:
    constexpr ShiftLight(LED_T& leds, const ShiftLightCfg<MIN_RPM_V, TARGET_RPM_V, BLINK_RPM_V>&) noexcept
     : _blinkTimer(80ms)
     , _leds(leds)
    { }

    constexpr void update(unsigned int rpm) noexcept
    {
        if (rpm >= _overrevTh) {
            _overrevTh = BLINK_RPM_V - hysteresis;
            _blinkTimer.poll([this]() noexcept { toggleLeds(); });
        } else {
            _overrevTh = BLINK_RPM_V;
            setLeds(rpm);
        }

        _leds.show();
    }

  private:
    static constexpr unsigned int numLeds = LED_T::numLeds;

    static constexpr unsigned int threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int stepSize = (TARGET_RPM_V - MIN_RPM_V) / (numLeds - 1);    // TODO correct?
        return MIN_RPM_V + stepSize * ledNo;                                             // TODO check
    }

    constexpr void toggleLeds() noexcept
    {
        for (unsigned int i = 0; i < numLeds; ++i) {
            _leds.setLed(i, _blinkState);
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

    unsigned int _overrevTh{BLINK_RPM_V};
    bool _blinkState{};
    PolledTimer _blinkTimer;
    LED_T& _leds;
};

template <typename LED_T, unsigned int MIN_RPM_V, unsigned int TARGET_RPM_V, unsigned int BLINK_RPM_V>
ShiftLight(LED_T&, const ShiftLightCfg<MIN_RPM_V, TARGET_RPM_V, BLINK_RPM_V>&)
    -> ShiftLight<LED_T, ShiftLightCfg<MIN_RPM_V, TARGET_RPM_V, BLINK_RPM_V>>;

#endif    // APP_INCLUDE_SHIFTLIGHT_HPP
