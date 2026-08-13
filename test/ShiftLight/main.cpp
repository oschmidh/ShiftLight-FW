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
    // return mp_units::value_cast<int>((1.0 / rate).in(mp_units::si::unit_symbols::us));
    // return mp_units::quantity_cast<mp_units::quantity<mp_units::si::unit_symbols::us, int>>(1.0 / rate);

    const auto f = (1.0 / rate);

    std::cout << "f (1.0 / rate):" << f.numerical_value_in(mp_units::si::micro<mp_units::si::second>) << " us\n";

    const auto fInt = mp_units::value_cast<int>(f.in(mp_units::si::unit_symbols::us));
    std::cout << "fInt:" << fInt.numerical_value_in(mp_units::si::micro<mp_units::si::second>) << " us\n";

    // return mp_units::value_cast<int>((1.0 / rate).in(mp_units::si::unit_symbols::us));

    // const mp_units::Quantity<mp_units::si::micro<mp_units::si::second>> q(fInt);
    const mp_units::quantity<mp_units::isq::time[mp_units::si::unit_symbols::us]> q(fInt);

    // static_assert(std::is_same_v<int, decltype(q)>);

    return fInt;
}

// template <unsigned int N_LEDS_V>
// static constexpr std::array<std::remove_cvref_t<decltype(minPeriod)>, N_LEDS_V> calculateThresholdsPeriod() noexcept
// {
//     using QuantityType = std::remove_cvref_t<decltype(minPeriod)>;

//     std::array<QuantityType, N_LEDS_V> thresholds;

//     const mp_units::quantity begin = 1.0 / minRate;
//     const mp_units::quantity end = 1.0 / targetRate;

//     for (std::size_t i = 0; i < thresholds.size(); ++i) {
//         thresholds[i] = value_cast<QuantityType>(begin + i * (end - begin) / (N_LEDS_V - 1.0));
//     }

//     return thresholds;
// }

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

// TEST_CASE("testing LED to period mapping")
// {
//     static constexpr unsigned int nLeds = 8;

//     FakeClock clock{};

//     EmulLeds<nLeds> leds;
//     ShiftLight shiftlight(leds, clock);

//     static constexpr std::array thresholds = calculateThresholdsPeriod<nLeds>();
//     // static constexpr auto stepsize = 5 * rpm;
//     static constexpr auto stepsize = 120 * mp_units::si::second;

//     SUBCASE("decreasing period")
//     {
//         for (auto period = 10000 * mp_units::si::milli<mp_units::si::second>; period < 1.0 / blinkRate;
//              period -= stepsize) {
//             shiftlight.update(period);
//             for (unsigned int i = 0; i < nLeds; ++i) {
//                 CHECK_MESSAGE(leds.isOn[i] == (period >= thresholds[i]), "failed for LED ", i, " at ",
//                               period.numerical_value_in(mp_units::si::milli<mp_units::si::second>), " RPM");
//             }
//         }
//     }

//     SUBCASE("increasing period")
//     {
//         for (auto period = 1.0 / blinkRate - stepsize; period > 0 * rpm; period -= stepsize) {
//             shiftlight.update(period);
//             for (unsigned int i = 0; i < nLeds; ++i) {
//                 CHECK_MESSAGE(leds.isOn[i] == (period >= thresholds[i]), "failed for LED ", i, " at ",
//                               period.numerical_value_in(mp_units::si::milli<mp_units::si::second>), "RPM");
//             }
//         }
//     }
// }

// static_assert(mp_units::value_cast<int>(1.0 / (5000 * rpm)) == 12 * mp_units::si::milli<mp_units::si::second>);

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
            // const auto period = toPeriod(rate);
            shiftlight.update(toPeriod(rate));
            for (unsigned int i = 0; i < nLeds; ++i) {
                CHECK_MESSAGE(
                    // leds.isOn[i] == (rate >= thresholds[i]), "failed for LED ", i, " at ",
                    // rate.numerical_value_in(rpm), " RPM (" <<
                    // period.numerical_value_in(mp_units::si::milli<mp_units::si::second>) << " ms)");
                    leds.isOn[i] == (rate >= thresholds[i]), "failed for LED ", i, " at ", rate.numerical_value_in(rpm),
                    " RPM");
            }
        }
    }

    SUBCASE("decreasing RPM")
    {
        for (auto rate = blinkRate - stepsize; rate > 0 * rpm; rate -= stepsize) {
            // shiftlight.update(mp_units::value_cast<1 * mp_units::si::micro<mp_units::si::second>, int>(1.0 / rate));
            // shiftlight.update(mp_units::value_cast<int>((1.0 / rate).in(mp_units::si::micro<mp_units::si::second>)));
            // shiftlight.update(mp_units::value_cast<int>((1.0 / rate).in(mp_units::si::unit_symbols::us)));
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
