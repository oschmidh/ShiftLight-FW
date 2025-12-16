#include "PolledTimer.hpp"

#include <testing/FakeClock.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <doctest/trompeloeil.hpp>

class ActionMock {
  public:
    MAKE_MOCK(action, auto()->void);
};

ActionMock mock;

void testAction() noexcept { mock.action(); }

using namespace std::literals::chrono_literals;

TEST_CASE("testing PolledTimer")
{
    FakeClock clock{};

    static constexpr auto period = 100ms;
    PolledTimer timer(clock, period);

    SUBCASE("no action if time is not elapsed")
    {
        timer.poll(testAction);
        clock.elapse(period - 10ms);
        timer.poll(testAction);
    }

    SUBCASE("action called after time elapsed")
    {
        REQUIRE_CALL(mock, action()).TIMES(1);

        clock.elapse(period);

        timer.poll(testAction);
    }

    SUBCASE("action called only once after time elapsed")
    {
        REQUIRE_CALL(mock, action()).TIMES(1);

        clock.elapse(period);

        timer.poll(testAction);
        timer.poll(testAction);
        timer.poll(testAction);
    }

    SUBCASE("test multiple elapsed cycles")
    {
        REQUIRE_CALL(mock, action()).TIMES(3);

        for (int i = 0; i < 3; ++i) {
            clock.elapse(period);
            timer.poll(testAction);
        }
    }
}
