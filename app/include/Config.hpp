#ifndef APP_INCLUDE_CONFIG_HPP
#define APP_INCLUDE_CONFIG_HPP

#include "LedBuffer.hpp"
#include "ShiftLight.hpp"

#include <experimental/array>

static constexpr unsigned int numLeds = 8;

static constexpr auto LedBufCfg = makeLedBufferConfig<{
    .numLeds = numLeds,
    .brightnessLut = std::experimental::make_array<std::uint8_t>(0xff, 0xff, 0xff,    // green
                                                                 0x26, 0x26, 0x26,    // yellow
                                                                 0x2e, 0x2e           // red
                                                                 ),
}>();

static constexpr auto shiftLightCfg = makeShiftLightConfig<{
    .minRpm = 4300,
    .targetRpm = 5700,
    .blinkRpm = 6000,
}>();

#endif    // APP_INCLUDE_CONFIG_HPP
