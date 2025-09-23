#ifndef SHIFTLIGHT_HPP
#define SHIFTLIGHT_HPP

static constexpr unsigned int minRpm = 4300;    // TODO better name
static constexpr unsigned int targetRpm = 5700;
static constexpr unsigned int blinkRpm = 6000;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRpm > minRpm);
static_assert(blinkRpm > targetRpm);

template <typename LED_T>
class ShiftLight {
  public:
    constexpr ShiftLight(LED_T& leds) noexcept
     : _leds(leds)
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

    LED_T& _leds;
};

#endif
