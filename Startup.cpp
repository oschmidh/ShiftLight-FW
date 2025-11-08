#include "Interrupt.hpp"

#include <array>
#include <cstdint>

// [[maybe_unused]] auto __attribute__((section(".intvecs"))) intVecTable = createInterruptTable<32>();

using FuncPtrType = void (*)();

// static_assert(sizeof(std::array<void (*)(), 32>) == sizeof(void (*)()[32]));
static_assert(sizeof(std::array<FuncPtrType, 32>) == sizeof(FuncPtrType[32]));

// [[maybe_unused]] std::array<void (*)(), 48> __attribute__((section(".intvecs"))) intVecTable<48>{
//     // TODO derive size
//     &__StackTop, resetHandler, nmiHandler, hardfaultHandler, nullptr, nullptr, nullptr, nullptr,
//     nullptr,     nullptr,      nullptr,    svcCallHandler,          nullptr, nullptr, pendSvHandler,  sysTickHandler
// };

// void (*const interruptVectors[])(void) __attribute__((used)) __attribute__((section(".intvecs"))) = {
//     (pFunc)&__StackTop, /* The initial stack pointer */
//     Reset_Handler,      /* The reset handler         */
//     NMI_Handler,        /* The NMI handler           */
//     HardFault_Handler,  /* The hard fault handler    */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     SVC_Handler,        /* SVCall handler            */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     PendSV_Handler,     /* The PendSV handler        */
//     SysTick_Handler,    /* SysTick handler           */
//     GROUP0_IRQHandler,  /* GROUP0 interrupt handler  */
//     GPIOA_IRQHandler,   /* GPIOA interrupt handler   */
//     TIMG8_IRQHandler,   /* TIMG8 interrupt handler   */
//     0,                  /* Reserved                  */
//     ADC0_IRQHandler,    /* ADC0 interrupt handler    */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     SPI0_IRQHandler,    /* SPI0 interrupt handler    */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     UART0_IRQHandler,   /* UART0 interrupt handler   */
//     TIMG14_IRQHandler,  /* TIMG14 interrupt handler  */
//     0,                  /* Reserved                  */
//     TIMA0_IRQHandler,   /* TIMA0 interrupt handler   */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     I2C0_IRQHandler,    /* I2C0 interrupt handler    */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     0,                  /* Reserved                  */
//     DMA_IRQHandler,     /* DMA interrupt handler     */

// };

//std::uint32_t stackTop{};

extern uint32_t __data_load__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __ramfunct_load__;
extern uint32_t __ramfunct_start__;
extern uint32_t __ramfunct_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;

typedef void (*pFunc)(void);

// extern void _init               (void) __attribute__((weak, alias("initStub")));

extern int main();
extern "C" void __libc_init_array(void);
extern "C" void _init(void) __attribute__((weak, alias("initStub")));    // TODO ??
extern "C" void initStub(void) { }                                       // TODO ??

constexpr void nmiHandler() noexcept { }
constexpr void hardfaultHandler() noexcept { }

constexpr void svcCallHandler() noexcept { }
constexpr void pendSvHandler() noexcept { }
constexpr void sysTickHandler() noexcept { }

constexpr void resetHandler() noexcept
{
    uint32_t *pui32Src, *pui32Dest;
    uint32_t *bs, *be;

    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pui32Src = &__data_load__;
    for (pui32Dest = &__data_start__; pui32Dest < &__data_end__;) {
        *pui32Dest++ = *pui32Src++;
    }

    //
    // Copy the ramfunct segment initializers from flash to SRAM.
    //
    pui32Src = &__ramfunct_load__;
    for (pui32Dest = &__ramfunct_start__; pui32Dest < &__ramfunct_end__;) {
        *pui32Dest++ = *pui32Src++;
    }

    // Initialize .bss to zero
    bs = &__bss_start__;
    be = &__bss_end__;
    while (bs < be) {
        *bs = 0;
        bs++;
    }

    /*
     * System initialization routine can be called here, but it's not
     * required for MSPM0.
     */
    // SystemInit();

    //
    // Initialize virtual tables, along executing init, init_array, constructors
    // and preinit_array functions
    //
    __libc_init_array();

    //
    // Call the application's entry point.
    //
    main();

    //
    // If we ever return signal Error
    //
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
