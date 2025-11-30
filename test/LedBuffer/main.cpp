#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "LedBuffer.hpp"
#include <doctest.h>
#include <trompeloeil.hpp>

struct ChannelBrightness {
        unsigned int channel;
        std::uint8_t duty;
    };

    constexpr bool operator==(const ChannelBrightness& lhs, const ChannelBrightness& rhs) noexcept {
        return lhs.channel == rhs.channel && lhs.duty == rhs.duty;
    }

class MockLedDriver
{
public:
    

  MAKE_CONST_MOCK(setBrightness, auto (ChannelBrightness) -> void);
};

TEST_CASE("testing single channel LedBuffer") {
    MockLedDriver drv;
    LedBuffer<MockLedDriver, 1> leds(drv);

    SUBCASE("setLed on"){
        leds.setLed(0, 0xff);
        leds.show();

        REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff}))  
        .TIMES(1);         
    }

    // SUBCASE("setLed off"){
    //     leds.setLed(0, 0);
    //     leds.show();

    //     REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0}))  
    //     .TIMES(1);         
    // } 

    SUBCASE("no update without call to show"){
        leds.setLed(0, 0xff);
        leds.setLed(0, 0);

        FORBID_CALL(drv, setBrightness(ANY(ChannelBrightness))); // TODO is already implicit?
    } 

    // SUBCASE("duplicate settings not updated"){
    //     leds.setLed(0, 0xff);
    //     leds.show();
    //     leds.setLed(0, 0xff);
    //     leds.show();

    //     REQUIRE_CALL(drv, setBrightness(ChannelBrightness{.channel = 0, .duty = 0xff}))  
    //     .TIMES(1);         
    // } 
}
