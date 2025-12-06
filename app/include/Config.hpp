// TODO include guards

#include "ShiftLight.hpp"

static constexpr auto shiftLightCfg = makeShiftLightConfig<{
    .minRpm = 4300,
    .targetRpm = 5700,
    .blinkRpm = 6000,
}>();
