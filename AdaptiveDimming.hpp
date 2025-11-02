#ifndef ADAPTIVEDIMMING_HPP
#define ADAPTIVEDIMMING_HPP

#include "MovingMean.hpp"

#include <cstdint>

template <typename SENSOR_T, typename DISPLAY_T>
class AdaptiveDimming {
  public:
    constexpr AdaptiveDimming(SENSOR_T& sens, DISPLAY_T& disp) noexcept
     : _sens(sens)
     , _disp(disp)
    { }

    constexpr void init() noexcept
    {
        _sens.fetch().transform(
            [this](SENSOR_T::ValueType brightness) noexcept { _filteredVal.init(std::move(brightness)); });

        setDayMode(_filteredVal.get() > dayThreshold);    // TODO should use conversion operator?
    }

    constexpr void update() noexcept
    {
        _sens.fetch().transform(
            [this](SENSOR_T::ValueType brightness) noexcept { _filteredVal.addSample(std::move(brightness)); });

        if (_dayMode) {
            if (_filteredVal.get() < nightThreshold) {    // TODO should use conversion operator?
                setDayMode(false);
            }

        } else {
            if (_filteredVal.get() > dayThreshold) {    // TODO should use conversion operator?
                setDayMode(true);
            }
        }
    }

  private:
    constexpr void setDayMode(bool status) noexcept
    {
        _dayMode = status;
        _disp.setGlobalBrightness(_dayMode ? dayModeBrightness : nightModeBrightness);    // TODO change name?
    }

    static constexpr std::uint8_t dayModeBrightness = 0x60;      // TODO make configurable?
    static constexpr std::uint8_t nightModeBrightness = 0x20;    // TODO make configurable?

    using BrightnessType = SENSOR_T::ValueType;

    static constexpr BrightnessType dayThreshold = 0xff;      // TODO make configurable?
    static constexpr BrightnessType nightThreshold = 0x1f;    // TODO make configurable?

    bool _dayMode = false;
    MovingMean<BrightnessType, 8> _filteredVal{};
    SENSOR_T& _sens;
    DISPLAY_T& _disp;
};

#endif    // ADAPTIVEDIMMING_HPP
