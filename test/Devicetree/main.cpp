#include "Devicetree.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <doctest/trompeloeil.hpp>

struct BasicDeviceA {
    int val{};
};

struct BasicDeviceB {
    bool val{};
};

struct BasicDeviceC {
    std::array<unsigned int, 32> vals{};
};

struct DependentDevice {
    DependentDevice(BasicDeviceA& parent)
     : parent(parent)
    { }

    BasicDeviceA& parent;
};

struct DeviceWithParam {
    DeviceWithParam(int a)
     : addr(a)
    { }

    int addr{};
};

TEST_CASE("testing basic isr table1")    // TODO rename
{
    // clang-format off
    using Dt = Devicetree<
        Node<
            BasicDeviceA,
            Label<"devA">
        >
    >;
    // clang-format on

    using DeviceContainerType = DeviceContainer<Dt>;
    DeviceContainerType devices{};

    // static_assert(sizeof(devices) == sizeof(BasicDeviceA)); // TODO ??

    devices.init();

    {
        auto& dev = devices.get<"devA">();
        CHECK(dev.val == 0);

        dev.val = 42;
    }

    {
        auto& dev = devices.get<"devA">();
        CHECK(dev.val == 42);
    }
}

TEST_CASE("testing basic isr table2")    // TODO rename
{
    // clang-format off
    using Dt = Devicetree<
        Node<
            BasicDeviceA,
            Label<"devA">
        >,

        Node<
            BasicDeviceB,
            Label<"devB">
        >,

        Node<
            BasicDeviceC,
            Label<"devC">
        >
    >;
    // clang-format on

    using DeviceContainerType = DeviceContainer<Dt>;
    DeviceContainerType devices{};

    devices.init();

    auto& devA = devices.get<"devA">();
    static_assert(std::is_same_v<decltype(devA), BasicDeviceA&>);

    auto& devB = devices.get<"devB">();
    static_assert(std::is_same_v<decltype(devB), BasicDeviceB&>);

    auto& devC = devices.get<"devC">();
    static_assert(std::is_same_v<decltype(devC), BasicDeviceC&>);
}

TEST_CASE("testing basic isr table3")    // TODO rename
{
    // clang-format off
    using Dt = Devicetree<
        Node<
            BasicDeviceA,
            Label<"dev0">
        >,

        Node<
            BasicDeviceA,
            Label<"dev1">
        >
    >;
    // clang-format on

    using DeviceContainerType = DeviceContainer<Dt>;
    DeviceContainerType devices{};

    // static_assert(sizeof(devices) == sizeof(BasicDeviceA) * 2); // TODO

    devices.init();

    auto& dev0 = devices.get<"dev0">();
    auto& dev1 = devices.get<"dev1">();

    CHECK(dev0.val == 0);
    CHECK(dev1.val == 0);

    dev0.val = 15;
    CHECK(dev0.val == 15);
    CHECK(dev1.val == 0);

    dev1.val = 9;
    CHECK(dev0.val == 15);
    CHECK(dev1.val == 9);
}

TEST_CASE("testing basic isr table4")    // TODO rename
{
    // clang-format off
    using Dt = Devicetree<
        Node<
            BasicDeviceA,
            Label<"parent">,

            Node<
                DependentDevice,
                Label<"child">
            >
        >
    >;
    // clang-format on

    using DeviceContainerType = DeviceContainer<Dt>;
    DeviceContainerType devices{};

    // static_assert(sizeof(devices) == sizeof(BasicDeviceA) * 2);

    devices.init();

    auto& parent = devices.get<"parent">();
    auto& child = devices.get<"child">();

    CHECK(parent.val == 0);
    CHECK(child.parent.val == 0);

    parent.val = 64;
    CHECK(parent.val == 64);
    CHECK(child.parent.val == 64);

    child.parent.val = 59;
    CHECK(parent.val == 59);
    CHECK(child.parent.val == 59);
}

TEST_CASE("testing basic isr table5")    // TODO rename
{
    // clang-format off
    using Dt = Devicetree<
        Node<
            DeviceWithParam,
            Label<"dev">,
            Address<0xab>
        >
    >;
    // clang-format on

    using DeviceContainerType = DeviceContainer<Dt>;
    DeviceContainerType devices{};

    devices.init();

    auto& dev = devices.get<"dev">();
    CHECK(dev.addr == 0xab);
}
