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
        _filteredVal.init(_sens.fetch());
        setDayMode(_filteredVal > dayThreshold);
    }

    constexpr void update() noexcept
    {
        _filteredVal << _sens.fetch();

        if (_dayMode) {
            if (_filteredVal < nightThreshold) {
                setDayMode(false);
            }

        } else {
            if (_filteredVal > dayThreshold) {
                setDayMode(true);
            }
        }
    }

  private:
    constexpr void setDayMode(bool status) noexcept
    {
        _dayMode = status;
        _disp.setBrightness(_dayMode ? dayModeBrightness : nightModeBrightness);
    }

    static constexpr std::uint8_t dayModeBrightness = 0xff;      // TODO make configurable?
    static constexpr std::uint8_t nightModeBrightness = 0x1f;    // TODO make configurable?

    using BrightnessType = SENSOR_T::ValueType;

    static constexpr BrightnessType dayThreshold = 0xff;      // TODO make configurable?
    static constexpr BrightnessType nightThreshold = 0x1f;    // TODO make configurable?

    bool _dayMode = false;
    MovingMean<BrightnessType, 8> _filteredVal{};
    SENSOR_T& _sens;
    DISPLAY_T& _disp;
};

#endif
