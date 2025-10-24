#ifndef LIB_INCLUDE_NVIC_HPP
#define LIB_INCLUDE_NVIC_HPP

#include <cstdint>
#include <array>

namespace nvic {

static constexpr unsigned int numCoreInterrupts = 15;

using IsrPtrType = void (*)();
static_assert(sizeof(std::array<IsrPtrType, 32>) == sizeof(IsrPtrType[32]));

template <std::size_t NUM_DEV_INTERRUPTS>
struct IntVecTable {
    std::uint32_t* stackPtr;
    std::array<IsrPtrType, numCoreInterrupts> coreIsrTable;
    std::array<IsrPtrType, NUM_DEV_INTERRUPTS> devIsrTable;
};

}    // namespace nvic

#endif    // LIB_INCLUDE_NVIC_HPP
