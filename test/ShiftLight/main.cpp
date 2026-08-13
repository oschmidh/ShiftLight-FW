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
static constexpr std::array<std::remove_cvref_t<decltype(minRate)>, N_LEDS_V> calculateThresholds() noexcept
{
    using QuantityType = std::remove_cvref_t<decltype(minRate)>;

    std::array<QuantityType, N_LEDS_V> thresholds;

    const mp_units::quantity begin = minRate;
    const mp_units::quantity end = targetRate;

    for (std::size_t i = 0; i < thresholds.size(); ++i) {
        thresholds[i] = value_cast<QuantityType>(begin + i * (end - begin) / (N_LEDS_V - 1.0));
    }

    return thresholds;
}

static constexpr mp_units::QuantityOf<mp_units::isq::time> auto toPeriod(
    mp_units::QuantityOf<mp_units::isq::frequency> auto rate) noexcept
{
    return mp_units::value_cast<int>((1.0 / rate).in(mp_units::si::unit_symbols::us));
}

using namespace std::literals::chrono_literals;

TEST_CASE("testing LED to RPM mapping")
{
    static constexpr unsigned int nLeds = 8;

    FakeClock clock{};

    EmulLeds<nLeds> leds;
    ShiftLight shiftlight(leds, clock);

    static constexpr std::array thresholds = calculateThresholds<nLeds>();
    static constexpr auto stepsize = 5 * rpm;

    SUBCASE("increasing RPM")
    {
        for (auto rate = 0 * rpm; rate < blinkRate; rate += stepsize) {
            shiftlight.update(rate);
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rate >= thresholds[i]), "failed for LED ", i, " at ",
                              rate.numerical_value_in(rpm), " RPM");
            }
        }
    }

    SUBCASE("decreasing RPM")
    {
        for (auto rate = blinkRate - stepsize; rate > 0 * rpm; rate -= stepsize) {
            shiftlight.update(rate);
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rate >= thresholds[i]), "failed for LED ", i, " at ",
                              rate.numerical_value_in(rpm), "RPM");
            }
        }
    }
}

TEST_CASE("testing LED to period mapping")
{
    static constexpr unsigned int nLeds = 8;

    FakeClock clock{};

    EmulLeds<nLeds> leds;
    ShiftLight shiftlight(leds, clock);

    static constexpr std::array thresholds = calculateThresholds<nLeds>();
    static constexpr auto stepsize = 5 * rpm;

    SUBCASE("increasing RPM")
    {
        for (auto rate = 0 * rpm; rate < blinkRate; rate += stepsize) {
            shiftlight.update(toPeriod(rate));
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rate >= thresholds[i]), "failed for LED ", i, " at ",
                              rate.numerical_value_in(rpm), " RPM");
            }
        }
    }

    SUBCASE("decreasing RPM")
    {
        for (auto rate = blinkRate - stepsize; rate > 0 * rpm; rate -= stepsize) {
            shiftlight.update(toPeriod(rate));
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(leds.isOn[i] == (rate >= thresholds[i]), "failed for LED ", i, " at ",
                              rate.numerical_value_in(rpm), "RPM");
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
        shiftlight.update(blinkRate - 1 * rpm);

        // all LEDs should be on by now
        REQUIRE(allLedsEqualTo(true));

        // LEDs should not be blinking already
        clock.elapse(blinkInterval);
        shiftlight.update(blinkRate - 1 * rpm);
        CHECK(allLedsEqualTo(true));

        // reaching the blink threshold should cause the LEDs to start blinking.
        // to reduce visual delay, the blinking shall start with an off-phase:
        shiftlight.update(blinkRate);
        CHECK(allLedsEqualTo(false));

        // the LEDs should stay off until the blinkIntervall passed
        clock.elapse(blinkInterval - 10ms);
        shiftlight.update(blinkRate);
        CHECK(allLedsEqualTo(false));

        // after the blinkInterval elapsed, the on-phase should begin:
        clock.elapse(10ms);
        shiftlight.update(blinkRate);
        CHECK(allLedsEqualTo(true));
    }

    SUBCASE("periodic blinking")
    {
        for (int i = 0; i < 32; ++i) {
            shiftlight.update(blinkRate);
            CHECK(allLedsEqualTo(i % 2));

            clock.elapse(blinkInterval + 1ms);
        }
    }

    SUBCASE("hysteresis")
    {
        shiftlight.update(blinkRate);

        // we should be in off-phase now:
        REQUIRE(allLedsEqualTo(false));

        // there shall be a hysteresis for the blink threshold:
        shiftlight.update(blinkRate - 25 * rpm);
        CHECK(allLedsEqualTo(false));

        // falling below the blink threshold (considering hysteresis) immediately stops the blinking (and turns all LEDs
        // back on):
        shiftlight.update(blinkRate - 100 * rpm);
        CHECK(allLedsEqualTo(true));

        // crossing the lower limit is not enought to start the blinking again:
        shiftlight.update(blinkRate - 1 * rpm);
        CHECK(allLedsEqualTo(true));

        // crossing the upper limit however immediately starts the blinking again:
        shiftlight.update(blinkRate);
        CHECK(allLedsEqualTo(false));
    }
}
