#include "ShiftLight.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <array>

template <unsigned int N_LEDS_V>
struct EmulLeds {
    static constexpr auto numLeds = std::integral_constant<unsigned int, N_LEDS_V>{};

    constexpr void setLed(unsigned int idx, bool state) noexcept
    {
        REQUIRE(idx < numLeds);
        _intermed[idx] = state;
    }

    constexpr void show() noexcept { std::copy(_intermed.begin(), _intermed.end(), isOn.begin()); }

    std::array<bool, numLeds> isOn{};

  private:
    std::array<bool, numLeds> _intermed{};
};

template <unsigned int N_LEDS_V>
static constexpr std::array<unsigned int, N_LEDS_V> calculateThresholds() noexcept
{
    std::array<unsigned int, N_LEDS_V> thresholds;

    const double begin = minRpm;
    const double end = targetRpm;

    for (std::size_t i = 0; i < thresholds.size(); ++i) {
        thresholds[i] = std::round(minRpm + i * (end - begin) / (N_LEDS_V - 1));
    }

    return thresholds;
}

TEST_CASE("")    // TODO add name
{
    static constexpr unsigned int nLeds = 4;

    EmulLeds<nLeds> leds;
    ShiftLight shiftlight(leds);

    static constexpr auto thresholds = calculateThresholds<nLeds>();

    SUBCASE("")    // TODO add name
    {
        for (unsigned int rpm = 0; rpm < blinkRpm; ++rpm) {
            shiftlight.update(rpm);
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rpm >= thresholds[i]), "failed for ", rpm, "RPM");
            }
        }
    }

    SUBCASE("")    // TODO add name
    {
        for (unsigned int rpm = blinkRpm; rpm > 0; --rpm) {
            shiftlight.update(rpm);
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rpm >= thresholds[i]), "failed for ", rpm, "RPM");
            }
        }
    }
}

// TODO add tests for overrev blinking
