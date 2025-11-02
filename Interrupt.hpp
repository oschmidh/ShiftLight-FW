#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP

#include "Delegate.hpp"

#include <array>

// class InterruptTable {    // TODO find better name
//   public:
//   private:
// };

// void (*const interruptVectors[])(void) __attribute__((used)) __attribute__((section(".intvecs"))) =
// IntVecTable

// static_assert(sizeof(std::array<void (*)(), 32>) == sizeof(void (*)()[32]));
// __attribute__((section(".intvecs"))) std::array<void (*)(), 32> intVecTable{};

namespace System {

class InterruptHandler {
  public:
    static constexpr unsigned int numLines = 32;
    using CallbackType = Delegate<void()>;

    static void registerIsr(unsigned int line, CallbackType isr) noexcept
    {
        if (line >= numLines) {
            return;
        }
        _callbacks[line] = isr;
    }

    // void enable(unsigned int line) noexcept { NVIC_EnableIRQ(); }

    // void disable(unsigned int line) noexcept { NVIC_DisableIRQ(); }

    template <unsigned int LINE_V>
        requires(LINE_V < numLines)
    static void isr() noexcept
    {
        _callbacks[LINE_V]();
    }

  private:
    inline static std::array<CallbackType, numLines> _callbacks{};
};

}    // namespace System

template <unsigned int NUM_LINES_V>
constexpr std::array<void (*)(), NUM_LINES_V> createInterruptTable() noexcept
{
    return []<unsigned int... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
        return std::array<void (*)(), NUM_LINES_V>{System::InterruptHandler::isr<IDX_Vs>...};
    }(std::make_index_sequence<NUM_LINES_V>());
}

#endif    // INTERRUPT_HPP
