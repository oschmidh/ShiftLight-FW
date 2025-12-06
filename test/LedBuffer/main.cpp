#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "LedBuffer.hpp"
#include <doctest.h>
#include <doctest/trompeloeil.hpp>

struct ChannelBrightness {
    unsigned int channel;
    std::uint8_t duty;
};

constexpr bool operator==(const ChannelBrightness& lhs, const ChannelBrightness& rhs) noexcept {
    return (lhs.channel == rhs.channel) && (lhs.duty == rhs.duty);
}

class MockLedDriver
{
  public:
    using BrightnessType = std::uint8_t;
    MAKE_CONST_MOCK(setBrightness, auto (ChannelBrightness) -> void);
};

TEST_CASE("testing single channel LedBuffer") {
    MockLedDriver drv;
    LedBuffer<MockLedDriver, 1> leds(drv);

    static_assert(leds.numLeds == 1);

    SUBCASE("setLed on"){
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff}))  
        .TIMES(1);         

        leds.setLed(0, true);
        leds.show();
    }

    SUBCASE("setLed off"){
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0}))  
        .TIMES(1);   

        leds.setLed(0, false);
        leds.show();
    }

    SUBCASE("no update without call to show"){
        leds.setLed(0, true);
        leds.setLed(0, false);
    }

    SUBCASE("no update without changes"){
        leds.show();
    }

    SUBCASE("duplicate settings not updated"){
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff}))  
        .TIMES(1);    

        leds.setLed(0, true);
        leds.show();
        leds.setLed(0, true);
        leds.show();
    }
}

TEST_CASE("testing multi channel LedBuffer") {
    MockLedDriver drv;
    LedBuffer<MockLedDriver, 4> leds(drv);

    static_assert(leds.numLeds == 4);

    SUBCASE("set all leds on"){
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff}))  
        .TIMES(1);       
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 1, .duty = 0xff}))  
        .TIMES(1); 
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 2, .duty = 0xff}))  
        .TIMES(1); 
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 3, .duty = 0xff}))  
        .TIMES(1);   

        for (unsigned int i=0; i< leds.numLeds; ++i) {
            leds.setLed(i, true);
        }
        leds.show();
    }

    SUBCASE("only modified leds are updated"){
        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 2, .duty = 0xff}))  
        .TIMES(1);    

        leds.setLed(2, true);
        leds.show();
    }
}

// TODO add tests for brightness lut
