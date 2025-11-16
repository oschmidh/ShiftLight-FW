#ifndef ADAPTIVEDIMMING_HPP
#define ADAPTIVEDIMMING_HPP

#include "MovingMean.hpp"

#include <cstdint>

#include "System.hpp"

#include <chrono>
#include <cstdint>

class PolledTimer {
  public:
    template <typename REP_T, typename PERIOD_T>
    PolledTimer(std::chrono::duration<REP_T, PERIOD_T> period) noexcept
     : _start(System::SteadyClock::now())
     , _period(std::chrono::duration_cast<System::SteadyClock::duration>(period))
    { }

    void poll(auto&& action) noexcept
    {
        if (!isElapsed()) {
            return;
        }
        reload();
        action();
    }

  private:
    bool isElapsed() const noexcept
    {
        const auto current = System::SteadyClock::now();
        return current - _start >= _period;
    }

    void reload() noexcept { _start = System::SteadyClock::now(); }

    System::SteadyClock::time_point _start;
    const System::SteadyClock::duration _period;
};

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
        auto res = _sens.fetch();
        if (!res.has_value()) {
            return;
        }

        _filteredVal.init(std::move(res.value()));
        setDayMode(_filteredVal.get() > dayThreshold);    // TODO should use conversion operator?
    }

    constexpr void run() noexcept
    {
        _updateTimer.poll([this]() noexcept { update(); });
    }

  private:
    constexpr void update() noexcept
    {
        auto res = _sens.fetch();
        if (!res.has_value()) {
            return;
        }

        _filteredVal.addSample(std::move(res.value()));

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

    constexpr void setDayMode(bool status) noexcept
    {
        _dayMode = status;
        _disp.setGlobalBrightness(_dayMode ? dayModeBrightness : nightModeBrightness);    // TODO change name?
    }

    static constexpr std::uint8_t dayModeBrightness = 0x50;      // TODO make configurable?
    static constexpr std::uint8_t nightModeBrightness = 0x10;    // TODO make configurable?

    using BrightnessType = SENSOR_T::ValueType;

    static constexpr BrightnessType dayThreshold = 0xff;      // TODO make configurable?
    static constexpr BrightnessType nightThreshold = 0x1f;    // TODO make configurable?

    bool _dayMode = false;
    MovingMean<BrightnessType, 8> _filteredVal{};
    PolledTimer _updateTimer{2s};
    SENSOR_T& _sens;
    DISPLAY_T& _disp;
};

#endif    // ADAPTIVEDIMMING_HPP
