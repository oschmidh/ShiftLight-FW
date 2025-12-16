#ifndef LIB_INCLUDE_TESTING_FAKECLOCK_HPP
#define LIB_INCLUDE_TESTING_FAKECLOCK_HPP

#include <chrono>

class FakeClock {
  public:
    using rep = std::chrono::steady_clock::rep;
    using period = std::chrono::steady_clock::period;
    using duration = std::chrono::steady_clock::duration;
    using time_point = std::chrono::steady_clock::time_point;
    static const bool is_steady = true;

    static time_point now() noexcept { return _elapsed; }

    template <typename REP_T, typename PERIOD_T>
    static void elapse(std::chrono::duration<REP_T, PERIOD_T> time) noexcept
    {
        if (time < duration{}) {
            return;
        }

        _elapsed += time;
    }

  private:
    inline static time_point _elapsed{};
};

#endif    // LIB_INCLUDE_TESTING_FAKECLOCK_HPP
