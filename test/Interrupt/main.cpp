#include "Interrupt.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <doctest/trompeloeil.hpp>

template <unsigned int INT_LINE_V>
class DeviceAWithInterrupt {
  public:
    static constexpr auto intLine = std::integral_constant<unsigned int, INT_LINE_V>{};

    MAKE_MOCK(isr, auto()->void);
};

template <unsigned int INT_LINE_V>
class DeviceBWithInterrupt {
  public:
    static constexpr auto intLine = std::integral_constant<unsigned int, INT_LINE_V>{};

    MAKE_MOCK(isr, auto()->void);
};

class DeviceWithoutInterrupt {
  public:
    MAKE_MOCK(isr, auto()->void);
};

DeviceAWithInterrupt<1> dev1;
// two devices on the same interrupt line:
DeviceAWithInterrupt<2> dev2;
DeviceBWithInterrupt<2> dev3;
DeviceWithoutInterrupt dev4;
DeviceAWithInterrupt<6> dev6;

TEST_CASE("testing basic isr table")
{
    static constexpr unsigned int numLines = 8;

    const auto isrTable = System::InterruptHandler<numLines, dev1, dev2, dev4, dev6>::createIsrTable();

    static_assert(isrTable.size() == numLines);

    SUBCASE("line with device called, isr executed")
    {
        REQUIRE_CALL(dev1, isr()).TIMES(1);

        isrTable[1]();
    }

    SUBCASE("line without device called, nothing executed")
    {
        isrTable[0]();
        isrTable[4]();
    }

    SUBCASE("call all lines in the table")
    {
        REQUIRE_CALL(dev1, isr()).TIMES(1);
        REQUIRE_CALL(dev2, isr()).TIMES(1);
        REQUIRE_CALL(dev6, isr()).TIMES(1);

        for (auto* isr : isrTable) {
            isr();
        }
    }
}

TEST_CASE("testing shared interrupts")
{
    static constexpr unsigned int numLines = 8;

    const auto isrTable = System::InterruptHandler<numLines, dev1, dev2, dev3>::createIsrTable();

    SUBCASE("line with multiple devices called, all isrs executed")
    {
        REQUIRE_CALL(dev2, isr()).TIMES(1);
        REQUIRE_CALL(dev3, isr()).TIMES(1);

        isrTable[2]();    // dev2 and dev3 share interrupt line 2
    }
}
