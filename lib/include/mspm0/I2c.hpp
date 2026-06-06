#ifndef LIB_INCLUDE_MSPM0_I2C_HPP
#define LIB_INCLUDE_MSPM0_I2C_HPP

#include "RegSet.hpp"

#include <span>
#include <utility>
#include <cstdint>

namespace mspm0 {

class I2c {
  public:
    I2c(std::uintptr_t addr) noexcept
     : _pwrCtrl(addr)
     , _clkCtrl(addr)
     , _intCtrl(addr, detail::regSet::intRegOffset)
     , _regs(new (reinterpret_cast<std::uint32_t*>(addr + regOffset)) Registers)
    { }

    void init() noexcept
    {
        _pwrCtrl.reset();
        _pwrCtrl.enable();

        _clkCtrl.setSource(detail::regSet::ClockControl2::ClockSource::BusClk);
    }

    struct GlitchFilterConfig {
        bool analogGlitchSuppression = true;
    };

    void configureGlitchFilter(const GlitchFilterConfig& cfg) noexcept
    {
        _regs->GFCTL = cfg.analogGlitchSuppression << 8u;
    }

    struct ControllerConfig {
        bool active = false;
        bool multiControllerMode = false;
        bool clockStretchDetection = false;
        bool loopbackTestmode = false;
    };

    std::span<const std::byte> fillTxFifo(std::span<const std::byte> data) noexcept
    {
        [[maybe_unused]] const volatile std::uintptr_t regAddr =
            reinterpret_cast<std::uintptr_t>(&_regs->controllerTxFifoStatus);
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

    class ControllerStatus {
      public:
        constexpr bool error() const volatile noexcept { return _reg & (1u << 1u); }
        constexpr bool idle() const volatile noexcept { return _reg & (1u << 5u); }
        constexpr bool busy() const volatile noexcept { return _reg & (1u << 6u); }
        constexpr unsigned int transactionCount() const volatile noexcept { return (_reg >> 16u) & 0xfff; }

      private:
        std::uint32_t _reg;    // TODO should this be volatile instead of the whole struct?
    };

    const volatile ControllerStatus& getControllerStatus() const volatile noexcept { return _regs->controllerStatus; }

    class FifoStatus {
      public:
        bool flushActive() const volatile noexcept { return _reg & 0x80; }
        unsigned int getCount() const volatile noexcept { return _reg & 0xf; }

      private:
        // volatile std::uint8_t _reg;
        std::uint8_t _reg;
    };

    const volatile FifoStatus& getControllerRxFifoStatus() const volatile noexcept
    {
        return _regs->controllerRxFifoStatus;
    }

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

    enum class Direction : std::uint32_t { Transmit = 0, Receive = 1 };

    void setControllerTargetAddr(std::uint8_t addr, Direction dir) noexcept
    {
        _regs->CSA = (addr << 1u) | std::to_underlying(dir);
    }

    struct ControllerOperationConfig {
        bool burstrun = false;
        bool generateStart = false;
        bool generateStop = false;
        bool autoAck = false;
        bool ackOverride = false;
        bool readOnTxEmpty = false;
        unsigned int transactionLength = 0;
    };

    void configureControllerOperation(const ControllerOperationConfig& cfg) noexcept
    {
        _regs->CCTR = (cfg.transactionLength << 16u) | (cfg.readOnTxEmpty << 5u) | (cfg.ackOverride << 4u) |
                      (cfg.autoAck << 3u) | (cfg.generateStop << 2u) | (cfg.generateStart << 1u) | cfg.burstrun;
    }

  private:
    class FifoControl {
      public:
        void setTrigger(unsigned int trigger) volatile noexcept
        {
            _reg &= ~trigBits;
            _reg |= trigger & trigBits;
        }

        void startFlush() noexcept { _reg |= flushBit; }
        void stopFlush() noexcept { _reg &= ~flushBit; }

      private:
        static constexpr std::uint8_t trigBits = 0x07;
        static constexpr std::uint8_t flushBit = 0x80;

        // volatile std::uint8_t _reg;
        std::uint8_t _reg;
    };

    detail::regSet::PowerControl _pwrCtrl;        // TODO dummy init param for test
    detail::regSet::ClockControl2 _clkCtrl;       // TODO dummy init param for test
    detail::regSet::InterruptControl _intCtrl;    // TODO dummy init param for test

    struct Registers {
        std::uint32_t GFCTL;
        std::uint32_t reserved_0[3];
        std::uint32_t CSA;
        std::uint32_t CCTR;
        ControllerStatus controllerStatus;
        std::uint32_t CRXDATA;
        std::uint32_t CTXDATA;
        std::uint32_t CTPR;
        std::uint32_t CCR;
        std::uint32_t reserved_1[2];
        std::uint32_t CBMON;
        FifoControl controllerRxFifoCtrl;
        FifoControl controllerTxFifoCtrl;
        std::uint16_t reserved_2;
        FifoStatus controllerRxFifoStatus;
        FifoStatus controllerTxFifoStatus;
        std::uint16_t reserved_3;
    };

    static constexpr uintptr_t regOffset = 0x1200;    // TODO fix

    volatile Registers* const _regs;
    // volatile Registers _regs;
};

}    // namespace mspm0

#endif // LIB_INCLUDE_MSPM0_I2C_HPP
