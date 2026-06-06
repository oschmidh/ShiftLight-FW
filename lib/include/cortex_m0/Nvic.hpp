#ifndef LIB_INCLUDE_CORTEX_M0_NVIC_HPP
#define LIB_INCLUDE_CORTEX_M0_NVIC_HPP

#include <cstdint>
#include <array>

namespace cortex_m0 {

namespace nvic {

// template <std::size_t N_DEV_INTERRUTPS_V>
// class Nvic {
// public:
static constexpr unsigned int numCoreInterrupts = 15;

using IsrPtrType = void (*)();
static_assert(sizeof(std::array<IsrPtrType, 32>) == sizeof(IsrPtrType[32]));

template <std::size_t NUM_DEV_INTERRUPTS>
struct IntVecTable {
    std::uint32_t* stackPtr;
    std::array<IsrPtrType, numCoreInterrupts> coreIsrTable;
    std::array<IsrPtrType, NUM_DEV_INTERRUPTS> devIsrTable;
};

inline void enableInterrupt(unsigned int irqNum) noexcept
{
    static constexpr std::uintptr_t ISER_addr = 0xe000e100;    // TODO somewhat ugly...
    *reinterpret_cast<std::uint32_t*>(ISER_addr) = 1u << irqNum;
}

inline void disableInterrupt(unsigned int irqNum) noexcept
{
    static constexpr std::uintptr_t ICER_addr = 0xe000e180;
    *reinterpret_cast<std::uint32_t*>(ICER_addr) = 1u << irqNum;
}

// private:

// };

}    // namespace nvic

}    // namespace cortex_m0

#endif    // LIB_INCLUDE_CORTEX_M0_NVIC_HPP
