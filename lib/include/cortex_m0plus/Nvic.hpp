#ifndef LIB_INCLUDE_CORTEX_M0PLUS_NVIC_HPP
#define LIB_INCLUDE_CORTEX_M0PLUS_NVIC_HPP

#include <cstdint>
#include <array>

namespace cortex_m0plus {

class Nvic {
  public:
    constexpr Nvic(std::uintptr_t addr) noexcept
     : _regs(new (reinterpret_cast<std::uint32_t*>(addr)) Registers)
    { }

    void enableInterrupt(unsigned int irqNum) noexcept { _regs->iser = 1u << irqNum; }

    void disableInterrupt(unsigned int irqNum) noexcept { _regs->icer = 1u << irqNum; }

  private:
    struct Registers {
        volatile std::uint32_t iser;
        std::uint32_t _reserved[31];
        volatile std::uint32_t icer;
    };

    Registers* _regs;
};

template <std::size_t N_DEV_INTERRUTPS_V>
struct IntVecTable {
    using IsrPtrType = void (*)();
    static_assert(sizeof(std::array<IsrPtrType, 32>) == sizeof(IsrPtrType[32]));

    static constexpr unsigned int nCoreInterrupts = 15;

    std::uint32_t* stackPtr;
    std::array<IsrPtrType, nCoreInterrupts> coreIsrTable;
    std::array<IsrPtrType, N_DEV_INTERRUTPS_V> devIsrTable;
};

}    // namespace cortex_m0plus

#endif    // LIB_INCLUDE_CORTEX_M0PLUS_NVIC_HPP
