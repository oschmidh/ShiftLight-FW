#ifndef SHIFTLIGHT_HPP
#define SHIFTLIGHT_HPP

static constexpr unsigned int minRpm = 4300;    // TODO better name
static constexpr unsigned int targetRpm = 5700;
static constexpr unsigned int blinkRpm = 6000;    // TODO auto derive from targetRpm + stepsize

static_assert(targetRpm > minRpm);
static_assert(blinkRpm > targetRpm);

#include "ti_msp_dl_config.h"
#include <cstdint>

class Timer {
  public:
    /* template <typename REP_T, typename PERIOD_T>
     Timer(std::chrono::duration<REP_T, PERIOD_T> period) noexcept
      : Timer(toTicks(period))
     { }*/

    Timer(unsigned int period) noexcept
     : _start(now())
     , _period(period)
    { }

    bool isElapsed() const noexcept
    {
        const auto current = now();
        if (_start > current) {    // handle overflow
            return current + DL_TimerA_getLoadValue(TIMA0) - _start >= _period;
        }

        return current - _start >= _period;
    }

    void reload() noexcept { _start = now(); }

    void handle(auto&& action) noexcept    // TODO find proper name
    {
        if (!isElapsed()) {
            return;
        }
        action();
        reload();
    }

  private:
    static std::uint32_t now() noexcept { return DL_TimerA_getTimerCount(TIMA0); }

    unsigned int _start;
    const unsigned int _period;
};

constexpr void periodicCall() noexcept { }

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
    Timer _blinkTimer{9000};
    LED_T& _leds;
};

#endif
