#ifndef APP_INCLUDE_SYSTEM_HPP
#define APP_INCLUDE_SYSTEM_HPP

#include <Devices.hpp>
#include <Clock.hpp>

#include <chrono>

namespace System {

using SteadyClock = TimerSteadyClock<Devices::sysTim>;

}    // namespace System

#endif    // APP_INCLUDE_SYSTEM_HPP
