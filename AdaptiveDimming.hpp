#ifndef ADAPTIVEDIMMING_HPP
#define ADAPTIVEDIMMING_HPP

#include "PolledTimer.hpp"

#include <cstdint>

using namespace std::literals::chrono_literals;

template <typename SENSOR_T, typename DISPLAY_T>
class AdaptiveDimming {
  public:
    constexpr AdaptiveDimming(SENSOR_T& sens, DISPLAY_T& disp) noexcept
     : _sens(sens)
     , _disp(disp)
    { }

    constexpr void init() noexcept
    {
        const auto brightness = _sens.fetch();
        if (!brightness.has_value()) {
            return;
        }

        setDayMode(brightness.value() > dayThreshold);
    }

    constexpr void run() noexcept
    {
        _updateTimer.poll([this]() noexcept { update(); });
    }

  private:
    constexpr void update() noexcept
    {
        const auto brightness = _sens.fetch();
        if (!brightness.has_value()) {
            return;
        }

        if (_dayMode) {
            if (brightness.value() < nightThreshold) {
                setDayMode(false);
            }

        } else {
            if (brightness.value() > dayThreshold) {
                setDayMode(true);
            }
        }
    }

    constexpr void setDayMode(bool status) noexcept
    {
        _dayMode = status;
        _disp.setBrightness(_dayMode ? dayModeBrightness : nightModeBrightness);
    }

    static constexpr DISPLAY_T::BrightnessType dayModeBrightness = 0x50;      // TODO make configurable?
    static constexpr DISPLAY_T::BrightnessType nightModeBrightness = 0x10;    // TODO make configurable?

    static constexpr SENSOR_T::ValueType dayThreshold = 0xff;      // TODO make configurable?
    static constexpr SENSOR_T::ValueType nightThreshold = 0x40;    // TODO make configurable?

    bool _dayMode = false;
    PolledTimer _updateTimer{2s};
    SENSOR_T& _sens;
    DISPLAY_T& _disp;
};

#endif    // ADAPTIVEDIMMING_HPP
