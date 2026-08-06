#ifndef APP_INCLUDE_SHIFTLIGHT_HPP
#define APP_INCLUDE_SHIFTLIGHT_HPP

#include "PolledTimer.hpp"

#include <mp-units/systems/si.h>

constexpr mp_units::Unit auto rpm = mp_units::mag_ratio<1, 60> * mp_units::si::hertz;
// inline constexpr struct rpm : mp_units::named_unit<"rpm", mp_units::mag<60> * mp_units::si::hertz> {
// } rpm;

static constexpr mp_units::quantity minRate = 4300 * rpm;    // TODO better name
static constexpr mp_units::quantity targetRate = 5700 * rpm;
static constexpr mp_units::quantity blinkRate = 6000 * rpm;    // TODO auto derive from targetRpm + stepsize

// static constexpr mp_units::quantity minPeriod = mp_units::value_cast<int>(1.0 / minRate);
// static constexpr mp_units::quantity targetPeriod = mp_units::value_cast<int>(1.0 / targetRate);
// static constexpr mp_units::quantity blinkPriod = mp_units::value_cast<int>(1.0 / blinkRate);

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

    template <auto U, typename T>
        requires mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::time> ||
                 mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::frequency>
    constexpr void update(mp_units::quantity<U, T> value) noexcept
    {
        if (_overreving) {
            if (belowBlinkThreshold(value)) {
                _overreving = false;
                setLeds(value);
            } else {
                _blinker.update(_leds);
            }

        } else {
            if (aboveBlinkThreshold(value)) {
                _overreving = true;
                _blinker.reset();    // reset blinker so it always starts with the same phase
                _blinker.blink(_leds);

            } else {
                setLeds(value);
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

    static constexpr bool aboveBlinkThreshold(mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
    {
        return rate >= blinkRate;
    }

    template <auto U, typename T>
        requires(mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::time>)
    static constexpr bool aboveBlinkThreshold(mp_units::quantity<U, T> period) noexcept
    {
        constexpr mp_units::quantity blinkPeriod = mp_units::value_cast<U, T>(1.0 / blinkRate);
        return period < blinkPeriod;
    }

    static constexpr bool belowBlinkThreshold(mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
    {
        return rate < (blinkRate - hysteresis);
    }

    template <auto U, typename T>
        requires(mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::time>)
    static constexpr bool belowBlinkThreshold(mp_units::quantity<U, T> period) noexcept
    {
        constexpr mp_units::quantity hysteresisPeriod = mp_units::value_cast<U, T>(1.0 / (blinkRate - hysteresis));
        return period >= (hysteresisPeriod);
    }

    static constexpr mp_units::QuantityOf<mp_units::isq::frequency> auto threshold(unsigned int ledNo) noexcept
    {
        constexpr unsigned int scaler = 1024;    // to reduce rounding error
        constexpr mp_units::quantity stepSize = (targetRate - minRate) * scaler / (numLeds - 1);
        return minRate + stepSize * ledNo / scaler;
    }

    template <auto U, typename T>
        requires(mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::frequency>)
    static constexpr bool belowLedThreshold(mp_units::quantity<U, T> rate, unsigned int nLed) noexcept
    {
        constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
            constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
            return std::array{mp_units::value_cast<U, T>(minRate + stepSize * IDX_Vs)...};
        }(std::make_index_sequence<numLeds>{});

        return rate < thresholds[nLed];
    }

    template <auto U, typename T>
        requires(mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::time>)
    static constexpr bool belowLedThreshold(mp_units::quantity<U, T> period, unsigned int nLed) noexcept
    {
        constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
            constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
            return std::array{mp_units::value_cast<U, T>(1.0 / (minRate + stepSize * IDX_Vs))...};
        }(std::make_index_sequence<numLeds>{});

        return period >= thresholds[nLed];
    }

    // constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    // return minRate + stepSize * ledNo;
    // }

    // template <mp_units::QuantityOf<mp_units::isq::frequency> QUANTITY_T>
    // static constexpr QUANTITY_T threshold(unsigned int ledNo) noexcept
    // {
    //     // constexpr unsigned int scaler = 1024;    // to reduce rounding error
    //     // constexpr mp_units::quantity stepSize = (targetRate - minRate) * scaler / (numLeds - 1);
    //     // return minRate + stepSize * ledNo / scaler;

    //     constexpr QUANTITY_T stepSize = (targetRate - minRate) / (numLeds - 1);
    //     return minRate + stepSize * ledNo;
    // }

    // constexpr void setLeds(mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
    // {
    //     // static constexpr std::array thresholds = []() {
    //     //     std::array<>;
    //     //      constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //     //     return minRate + stepSize * i;
    //     // }();

    //     // static constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //     //     constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //     //     return std::array{(minRate + stepSize * IDX_Vs)...};
    //     // }(std::make_index_sequence<numLeds>{});

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (rate < threshold(i)) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    // template <mp_units::QuantityOf<mp_units::isq::frequency> QUANTITY_T>
    // constexpr void setLeds(QUANTITY_T rate) noexcept
    // {

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (rate < threshold<QUANTITY_T>(i)) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    // constexpr void setLeds(mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
    // {
    //     static constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //         constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //         return std::array{(minRate + stepSize * IDX_Vs)...};
    //     }(std::make_index_sequence<numLeds>{});

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (rate < thresholds[i]) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    // template <mp_units::QuantityOf<mp_units::isq::time> QUANTITY_T>
    // constexpr void setLeds(QUANTITY_T period) noexcept

    // template <auto U, typename T>
    //     requires(mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::time>)
    // constexpr void setLeds(mp_units::quantity<U, T> period) noexcept
    // {
    //     constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //         constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //         return std::array{mp_units::value_cast<U, T>(1.0 / (minRate + stepSize * IDX_Vs))...};
    //         // return std::array{mp_units::value_cast<int>(minPeriod + stepSize * IDX_Vs)...};
    //     }(std::make_index_sequence<numLeds>{});

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (period >= thresholds[i]) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    template <auto U, typename T>
        requires mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::time> ||
                 mp_units::QuantityOf<mp_units::quantity<U, T>, mp_units::isq::frequency>
    constexpr void setLeds(mp_units::quantity<U, T> value) noexcept
    {
        unsigned int i = 0;
        for (; i < numLeds; ++i) {
            if (belowLedThreshold(value, i)) {
                break;
            }

            _leds.setLed(i, true);
        }

        for (; i < numLeds; ++i) {
            _leds.setLed(i, false);
        }
    }

    // template <mp_units::QuantityOf<mp_units::isq::frequency> QUANTITY_T>
    // constexpr void setLeds(QUANTITY_T rate) noexcept
    // {
    //     // static constexpr std::array<QUANTITY_T, numLeds> thresholds = []() {
    //     //     std::array<QUANTITY_T, numLeds> ths;
    //     //     constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //     //     for (std::size_t i = 0; i < ths.size(); ++i) {
    //     //         ths[i] = minRate + stepSize * i;
    //     //     }
    //     //     return ths;
    //     // }();

    //     // static constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //     //     constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //     //     return std::array{value_cast<QUANTITY_T>(minRate + stepSize * IDX_Vs)...};
    //     // }(std::make_index_sequence<numLeds>{});

    //     static constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //         static constexpr unsigned int scaler = 1024;
    //         static constexpr mp_units::quantity stepSize = (targetRate - minRate) * scaler / (numLeds - 1);
    //         return std::array{value_cast<QUANTITY_T>(minRate + stepSize * IDX_Vs / scaler)...};
    //     }(std::make_index_sequence<numLeds>{});

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (rate < thresholds[i]) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    // template <mp_units::QuantityOf<mp_units::isq::frequency> QUANTITY_T>
    // constexpr void setLeds(QUANTITY_T rate) noexcept
    // {
    //     // static constexpr std::array thresholds = []() {
    //     //     std::array<>;
    //     //      constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //     //     return minRate + stepSize * i;
    //     // }();

    //     // static constexpr std::array thresholds = []<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) {
    //     //     // std::array<>;
    //     //     constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //     //     return std::array{({minRate + stepSize * IDX_Vs}.in(QUANTITY_T))...};
    //     // }(std::make_index_sequence<numLeds>{});

    //     static constexpr std::array<QUANTITY_T, numLeds> thresholds = []() {
    //         std::array<QUANTITY_T, numLeds> ths;
    //         constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //         for (std::size_t i = 0; i < ths.size(); ++i) {
    //             ths[i] = minRate + stepSize * i;
    //         }
    //         return ths;
    //     }();

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (rate < thresholds[i]) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    // template<mp_units::QuantityOf<mp_units::isq::frequency> QUANTITY_T>
    // constexpr void setLeds(QUANTITY_T rate) noexcept
    // {
    //     static constexpr std::array thresholds = [](){
    //         std::array<QUANTITY_T, >()
    //         constexpr mp_units::quantity stepSize = (targetRate - minRate) / (numLeds - 1);
    //         return minRate + stepSize * ledNo;
    //     }();

    //     unsigned int i = 0;
    //     for (; i < numLeds; ++i) {
    //         if (rate < threshold(i)) {
    //             break;
    //         }

    //         _leds.setLed(i, true);
    //     }

    //     for (; i < numLeds; ++i) {
    //         _leds.setLed(i, false);
    //     }
    // }

    static constexpr auto hysteresis = 50 * rpm;
    // static constexpr auto hysteresisPeriod = mp_units::value_cast<int>(1.0 / hysteresis);

    bool _overreving{};
    Blinker _blinker;
    LED_T& _leds;
};

#endif    // APP_INCLUDE_SHIFTLIGHT_HPP
