#include "LedBuffer.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <doctest/trompeloeil.hpp>

struct ChannelBrightness {
    unsigned int channel;
    std::uint8_t duty;
};

constexpr bool operator==(const ChannelBrightness& lhs, const ChannelBrightness& rhs) noexcept
{
    return (lhs.channel == rhs.channel) && (lhs.duty == rhs.duty);
}

class MockLedDriver {
  public:
    using BrightnessType = std::uint8_t;
    MAKE_CONST_MOCK(setBrightness, auto(ChannelBrightness)->void);
};

TEST_CASE("testing single channel LedBuffer")
{
    MockLedDriver drv;
    LedBuffer<MockLedDriver, 1> leds(drv);

    static_assert(leds.numLeds == 1);

    {    // initialize ledBuf to zero
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0})).TIMES(1);
        leds.show();
    }

    SUBCASE("setLed on/off")
    {
        trompeloeil::sequence seq;

        {
            REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff})).TIMES(1);

            leds.setLed(0, true);
            leds.show();
        }

        {
            REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0})).TIMES(1);

            leds.setLed(0, false);
            leds.show();
        }
    }

    SUBCASE("no update without call to show")
    {
        leds.setLed(0, true);
        leds.setLed(0, false);
    }

    SUBCASE("no update without changes") { leds.show(); }

    SUBCASE("duplicate settings not updated")
    {
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff})).TIMES(1);

        leds.setLed(0, true);
        leds.show();
        leds.setLed(0, true);
        leds.show();
    }
}

TEST_CASE("testing multi channel LedBuffer")
{
    MockLedDriver drv;
    LedBuffer<MockLedDriver, 4> leds(drv);

    static_assert(leds.numLeds == 4);

    {    // initialize ledBuf to zero
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0})).TIMES(1);
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 1, .duty = 0})).TIMES(1);
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 2, .duty = 0})).TIMES(1);
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 3, .duty = 0})).TIMES(1);
        leds.show();
    }

    SUBCASE("set all leds on")
    {
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff})).TIMES(1);
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 1, .duty = 0xff})).TIMES(1);
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 2, .duty = 0xff})).TIMES(1);
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 3, .duty = 0xff})).TIMES(1);

        for (unsigned int i = 0; i < leds.numLeds; ++i) {
            leds.setLed(i, true);
        }
        leds.show();
    }

    SUBCASE("only modified leds are updated")
    {
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 2, .duty = 0xff})).TIMES(1);

        leds.setLed(2, true);
        leds.show();
    }
}

TEST_CASE("testing custom brightness table")
{
    MockLedDriver drv;
    static constexpr std::array<MockLedDriver::BrightnessType, 3> brightnessTable{0xaa, 0xbb, 0xcc};
    LedBuffer<MockLedDriver, 3, brightnessTable> leds(drv);

    REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xaa})).TIMES(1);
    REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 1, .duty = 0xbb})).TIMES(1);
    REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 2, .duty = 0xcc})).TIMES(1);

    for (unsigned int i = 0; i < leds.numLeds; ++i) {
        leds.setLed(i, true);
    }
    leds.show();
}
