#include "ShiftLight.hpp"

#include <testing/FakeClock.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

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

using namespace std::literals::chrono_literals;

TEST_CASE("testing LED to RPM mapping")
{
    static constexpr unsigned int nLeds = 8;

    FakeClock clock{};

    EmulLeds<nLeds> leds;
    ShiftLight shiftlight(leds, clock);

    static constexpr auto thresholds = calculateThresholds<nLeds>();
    static constexpr unsigned int stepsize = 50;

    SUBCASE("increasing RPM")
    {
        for (unsigned int rpm = 0; rpm < blinkRpm; rpm += stepsize) {
            shiftlight.update(rpm);
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rpm >= thresholds[i]), "failed for LED ", i, " at ", rpm, "RPM");
            }
        }
    }

    SUBCASE("decreasing RPM")
    {
        for (unsigned int rpm = blinkRpm - stepsize; rpm > 0; rpm -= stepsize) {
            shiftlight.update(rpm);
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rpm >= thresholds[i]), "failed for LED ", i, " at ", rpm, "RPM");
            }
        }
    }
}

TEST_CASE("testing LED blinking when overreving")
{
    static constexpr unsigned int nLeds = 4;

    FakeClock clock{};

    EmulLeds<nLeds> leds;
    ShiftLight shiftlight(leds, clock);

    const auto allLedsEqualTo = [&leds](bool expected) -> bool {
        for (unsigned int i = 0; i < nLeds; ++i) {
            if (leds.isOn[i] != expected) {
                return false;
            }
        }
        return true;
    };

    SUBCASE("crossing threshold starts blinking")
    {
        shiftlight.update(blinkRpm - 1);

        // all LEDs should be on by now
        REQUIRE(allLedsEqualTo(true));

        // LEDs should not be blinking already
        clock.elapse(blinkInterval);
        shiftlight.update(blinkRpm - 1);
        CHECK(allLedsEqualTo(true));

        // reaching the blink threshold should cause the LEDs to start blinking.
        // to reduce visual delay, the blinking shall start with an off-phase:
        shiftlight.update(blinkRpm);
        CHECK(allLedsEqualTo(false));

        // the LEDs should stay off until the blinkIntervall passed
        clock.elapse(blinkInterval - 10ms);
        shiftlight.update(blinkRpm);
        CHECK(allLedsEqualTo(false));

        // after the blinkInterval elapsed, the on-phase should begin:
        clock.elapse(10ms);
        shiftlight.update(blinkRpm);
        CHECK(allLedsEqualTo(true));
    }

    SUBCASE("periodic blinking")
    {
        for (int i = 0; i < 32; ++i) {
            shiftlight.update(blinkRpm);
            CHECK(allLedsEqualTo(i % 2));

            clock.elapse(blinkInterval + 1ms);
        }
    }

    SUBCASE("hysteresis")
    {
        shiftlight.update(blinkRpm);

        // we should be in off-phase now:
        REQUIRE(allLedsEqualTo(false));

        // there shall be a hysteresis for the blink threshold:
        shiftlight.update(blinkRpm - 25);
        CHECK(allLedsEqualTo(false));

        // falling below the blink threshold (considering hysteresis) immediately stops the blinking (and turns all LEDs
        // back on):
        shiftlight.update(blinkRpm - 100);
        CHECK(allLedsEqualTo(true));

        // crossing the lower limit is not enought to start the blinking again:
        shiftlight.update(blinkRpm - 1);
        CHECK(allLedsEqualTo(true));

        // crossing the upper limit however immediately starts the blinking again:
        shiftlight.update(blinkRpm);
        CHECK(allLedsEqualTo(false));
    }
}
