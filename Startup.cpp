#include "Interrupt.hpp"

#include <array>
#include <algorithm>
#include <cstdint>

using FuncPtrType = void (*)();

static_assert(sizeof(std::array<FuncPtrType, 32>) == sizeof(FuncPtrType[32]));

extern uint32_t __data_load__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __ramfunct_load__;
extern uint32_t __ramfunct_start__;
extern uint32_t __ramfunct_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;

extern int main();
extern "C" void __libc_init_array(void);
extern "C" void _init(void) __attribute__((weak, alias("initStub")));    // TODO ??
extern "C" void initStub(void) { }                                       // TODO ??

[[noreturn]] constexpr void hardfaultHandler() noexcept
{
    while (1) {
    }
}

constexpr void nmiHandler() noexcept { }
constexpr void svcCallHandler() noexcept { }
constexpr void pendSvHandler() noexcept { }
constexpr void sysTickHandler() noexcept { }

[[noreturn]] constexpr void resetHandler() noexcept
{
    // Copy data initializers from flash to SRAM
    std::copy_n(&__data_load__, __data_end__ - __data_start__, &__data_start__);

    // zero bss section
    std::fill(&__bss_start__, &__bss_end__, 0);

    __libc_init_array();

    main();

    hardfaultHandler();
}

struct InterrupVectorTable {
    std::uint32_t* stackPtr = &__StackTop;
    std::array<void (*)(), 15> intVecTable{// TODO derive size
                                           resetHandler,   nmiHandler, hardfaultHandler, nullptr,       nullptr,
                                           nullptr,        nullptr,    nullptr,          nullptr,       nullptr,
                                           svcCallHandler, nullptr,    nullptr,          pendSvHandler, sysTickHandler};
    std::array<void (*)(), 32> deviceInterrupts = createInterruptTable<32>();
};

[[gnu::section(".intvecs"), gnu::used]] InterrupVectorTable intVecTable{};
