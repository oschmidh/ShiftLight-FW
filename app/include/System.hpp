#ifndef APP_INCLUDE_SYSTEM_HPP
#define APP_INCLUDE_SYSTEM_HPP

#include <Devices.hpp>
#include <TimerClock.hpp>

namespace System {

using SteadyClock = TimerSteadyClock<Devices::timA0>;

}    // namespace System

#endif // APP_INCLUDE_SYSTEM_HPP
