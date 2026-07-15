#ifndef LIB_INCLUDE_MSPM0_I2C_HPP
#define LIB_INCLUDE_MSPM0_I2C_HPP

#include "CommonRegs.hpp"

#include <span>
#include <utility>
#include <cstdint>

namespace mspm0 {

class I2c : detail::Peripheral<I2c> {
  public:
    enum class ClockSource : std::uint32_t {
        BusClk = 1 << 3u,
        MfClk = 1 << 2u,
    };

    struct GlitchFilterConfig {
        bool analogGlitchSuppression = true;
    };

    struct ControllerConfig {
        bool active = false;
        bool multiControllerMode = false;
        bool clockStretchDetection = false;
        bool loopbackTestmode = false;
    };

    class ControllerStatus {
      public:
        constexpr bool error() const noexcept { return _reg & (1u << 1u); }
        constexpr bool idle() const noexcept { return _reg & (1u << 5u); }
        constexpr bool busy() const noexcept { return _reg & (1u << 6u); }
        constexpr unsigned int transactionCount() const noexcept { return (_reg >> 16u) & 0xfff; }

      private:
        volatile std::uint32_t _reg;
    };

    class FifoStatus {
      public:
        bool flushActive() const noexcept { return _reg & 0x80; }
        unsigned int getCount() const noexcept { return _reg & 0xf; }

      private:
        volatile std::uint8_t _reg;
    };

    enum class Direction : std::uint32_t { Transmit = 0, Receive = 1 };

    struct ControllerOperationConfig {
        bool burstrun = false;
        bool generateStart = false;
        bool generateStop = false;
        bool autoAck = false;
        bool ackOverride = false;
        bool readOnTxEmpty = false;
        unsigned int transactionLength = 0;
    };

    struct Interrupts {
        enum class InterruptVals : std::uint32_t { };
    };    // TODO make not needed...

    I2c(std::uintptr_t addr) noexcept
     : detail::Peripheral<I2c>(addr)
     //  , _intCtrl(addr)
     , _clkRegs(new (reinterpret_cast<std::uint32_t*>(addr + clockRegOffset)) ClockRegisters)
     , _regs(new (reinterpret_cast<std::uint32_t*>(addr + regOffset)) Registers)
    { }

    void init() noexcept
    {
        _pwrRegs->resetCtl.reset();
        _pwrRegs->powerEn.enable();

        _clkRegs->clockSel.setSource(ClockSource::BusClk);
    }

      void configureGlitchFilter(const GlitchFilterConfig& cfg) noexcept
    {
        _regs->GFCTL = cfg.analogGlitchSuppression << 8u;
    }

    std::span<const std::byte> fillTxFifo(std::span<const std::byte> data) noexcept
    {
        // [[maybe_unused]] const volatile std::uintptr_t regAddr =
        //     reinterpret_cast<std::uintptr_t>(&_regs->controllerTxFifoStatus);
        // [[maybe_unused]] const volatile cfifosr = *static_casty<std::uint32_t*>(0x4);

        while (_regs->controllerTxFifoStatus.getCount() && !data.empty()) {
            _regs->CTXDATA = std::to_integer<std::uint32_t>(data[0]);
            data = data.subspan<1>();
        }
        return data;
    }

    std::span<std::byte> readRxFifo(std::span<std::byte> data) const noexcept
    {
        while (_regs->controllerRxFifoStatus.getCount() && !data.empty()) {
            data[0] = std::byte{static_cast<std::uint8_t>(_regs->CRXDATA)};
            data = data.subspan<1>();
        }
        return data;
    }

    void setControllerTimerPeriod(unsigned int period) noexcept { _regs->CTPR = period; }

    const ControllerStatus& getControllerStatus() const noexcept { return _regs->controllerStatus; }

        const FifoStatus& getControllerRxFifoStatus() const noexcept { return _regs->controllerRxFifoStatus; }

    void configureController(const ControllerConfig& cfg) noexcept
    {
        _regs->CCR = (cfg.loopbackTestmode << 8u) | (cfg.clockStretchDetection << 2u) |
                     (cfg.multiControllerMode << 1u) | cfg.active;
    }

    // max 7 bytes
    // TODO enforce with enum?
    void setControllerTxFifoTriggerByteLevel(unsigned int byteThreshold) noexcept
    {
        _regs->controllerTxFifoCtrl.setTrigger(byteThreshold);
    }

    // max 7 bytes
    // TODO enforce with enum?
    void setControllerRxFifoTriggerByteLevel(unsigned int byteThreshold) noexcept
    {
        _regs->controllerRxFifoCtrl.setTrigger(byteThreshold);
    }

    void setControllerTargetAddr(std::uint8_t addr, Direction dir) noexcept
    {
        _regs->CSA = (addr << 1u) | std::to_underlying(dir);
    }

    void configureControllerOperation(const ControllerOperationConfig& cfg) noexcept
    {
        _regs->CCTR = (cfg.transactionLength << 16u) | (cfg.readOnTxEmpty << 5u) | (cfg.ackOverride << 4u) |
                      (cfg.autoAck << 3u) | (cfg.generateStop << 2u) | (cfg.generateStart << 1u) | cfg.burstrun;
    }

  private:
    class FifoControl {
      public:
        void setTrigger(unsigned int trigger) noexcept
        {
            _reg &= ~trigBits;
            _reg |= trigger & trigBits;
        }

        void startFlush() noexcept { _reg |= flushBit; }
        void stopFlush() noexcept { _reg &= ~flushBit; }

      private:
        static constexpr std::uint8_t trigBits = 0x07;
        static constexpr std::uint8_t flushBit = 0x80;

        volatile std::uint8_t _reg;
    };

    struct ClockRegisters {
        detail::commonRegs::ClockDiv clockDiv;
        detail::commonRegs::ClockSel<ClockSource> clockSel;
    };

    struct Registers {    // TODO rename I2cRegs?
        volatile std::uint32_t GFCTL;
        volatile std::uint32_t reserved_0[3];
        volatile std::uint32_t CSA;
        volatile std::uint32_t CCTR;
        ControllerStatus controllerStatus;
        volatile std::uint32_t CRXDATA;
        volatile std::uint32_t CTXDATA;
        volatile std::uint32_t CTPR;
        volatile std::uint32_t CCR;
        volatile std::uint32_t reserved_1[2];
        volatile std::uint32_t CBMON;
        FifoControl controllerRxFifoCtrl;
        FifoControl controllerTxFifoCtrl;
        volatile std::uint16_t reserved_2;
        FifoStatus controllerRxFifoStatus;
        FifoStatus controllerTxFifoStatus;
        volatile std::uint16_t reserved_3;
    };

    static constexpr uintptr_t clockRegOffset = 0x1000;
    static constexpr uintptr_t regOffset = 0x1200;    // TODO fix

    // detail::regSet::InterruptEventControl _intCtrl;

    ClockRegisters* const _clkRegs;
    Registers* const _regs;
};

}    // namespace mspm0

#endif // LIB_INCLUDE_MSPM0_I2C_HPP
