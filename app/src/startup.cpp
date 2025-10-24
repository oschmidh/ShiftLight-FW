#include "Devices.hpp"
#include "Interrupt.hpp"
#include "Nvic.hpp"

#include <array>
#include <span>
#include <algorithm>
#include <cstdint>

extern const std::uint32_t __data_load__;
extern std::uint32_t __data_start__;
extern std::uint32_t __data_end__;
extern const std::uint32_t __ramfunct_load__;
extern std::uint32_t __ramfunct_start__;
extern std::uint32_t __ramfunct_end__;
extern std::uint32_t __bss_start__;
extern std::uint32_t __bss_end__;
extern std::uint32_t __StackTop;

extern int main();

extern "C" void __libc_init_array(void);
extern "C" void _init(void) { }

[[noreturn]] constexpr void hardfaultHandler() noexcept
{
    while (1)
        ;
}

constexpr void nmiHandler() noexcept { }
constexpr void svcCallHandler() noexcept { }
constexpr void pendSvHandler() noexcept { }
constexpr void sysTickHandler() noexcept { }

constexpr void initDataSection() noexcept
{
    const std::span ramData = {&__data_start__, &__data_end__};
    const std::span flashData = {&__data_load__, ramData.size()};
    std::copy(flashData.begin(), flashData.end(), ramData.begin());
}

constexpr void copyRamfuncs() noexcept
{
    const std::span ramFuncRam = {&__ramfunct_start__, &__ramfunct_end__};
    const std::span ramFuncFlash = {&__ramfunct_load__, ramFuncRam.size()};
    std::copy(ramFuncFlash.begin(), ramFuncFlash.end(), ramFuncRam.begin());
}

constexpr void initBssSection() noexcept
{
    const std::span bss = {&__bss_start__, &__bss_end__};
    std::ranges::fill(bss, 0);
}

[[noreturn]] constexpr void resetHandler() noexcept
{
    initDataSection();

    copyRamfuncs();

    initBssSection();

    __libc_init_array();

    main();

    hardfaultHandler();
}

static constexpr unsigned int numDevInterrupts = 32;

[[gnu::section(".intvecs"), gnu::used]] constinit const nvic::IntVecTable intVecTable{
    .stackPtr = &__StackTop,
    .coreIsrTable =
        {
            resetHandler,
            nmiHandler,
            hardfaultHandler,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            svcCallHandler,
            nullptr,
            nullptr,
            pendSvHandler,
            sysTickHandler,
        },
    .devIsrTable =
        System::InterruptHandler<numDevInterrupts, Devices::i2c0, Devices::timG8, Devices::timA0>::createIsrTable(),
};
