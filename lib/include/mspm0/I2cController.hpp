#ifndef LIB_INCLUDE_MSPM0_I2CCONTROLLER_HPP
#define LIB_INCLUDE_MSPM0_I2CCONTROLLER_HPP

#include "I2c.hpp"

#include <span>
#include <utility>
#include <cstdint>

enum class I2cError {
    NoError,
    IoError,
    InvalidParam,
};

namespace mspm0 {

class I2cController {
  public:
    using ErrorType = I2cError;

    I2cController(I2c& i2c) noexcept
     : _i2c(i2c)
    { }

    void init() noexcept
    {
        _i2c.init();

        _i2c.configureGlitchFilter({.analogGlitchSuppression = false});    // TODO needed?

        // TODO make configurable at compile time:
        static constexpr unsigned int i2cClk = 24'000'000;
        static constexpr unsigned int i2cFreq = 100'000;
        static_assert(i2cClk >= 20 * i2cFreq, "requirement in refMan");    // TODO edit message
        static constexpr unsigned int sclLp = 6;
        static constexpr unsigned int sclHp = 4;
        static constexpr unsigned int tpr = (i2cClk / (i2cFreq * (sclLp + sclHp))) - 1;
        static_assert(tpr <= 0x7F);

        _i2c.setControllerTimerPeriod(tpr);

        _i2c.setControllerTxFifoTrigger(I2c::ControllerTxFifoThresh::EqualOrMoreThan2);
        _i2c.setControllerRxFifoTrigger(I2c::ControllerRxFifoThresh::WhenEmpty);

        _i2c.configureController({.active = true, .clockStretchDetection = true});
    }

    ErrorType write(std::uint8_t addr, std::span<const std::byte> data) noexcept
    {
        const auto size = data.size();
        if (size > 0xfff) {    // TODO unlikely?
            return ErrorType::InvalidParam;
        }

        while (!_i2c.getControllerStatus().idle())
            ;

        data = _i2c.fillTxFifo(data);

        _i2c.setControllerTargetAddr(addr, I2c::Direction::Transmit);
        _i2c.configureControllerOperation({.burstrun = true,
                                           .generateStart = true,
                                           .generateStop = true,
                                           .readOnTxEmpty = false,
                                           .transactionLength = size});

        while (!data.empty()) {
            data = _i2c.fillTxFifo(data);
        }

        // Poll until the Controller writes all bytes
        while (_i2c.getControllerStatus().busy())
            ;

        if (_i2c.getControllerStatus().error()) {
            DL_I2C_flushControllerTXFIFO(I2C0);
            return ErrorType::IoError;
        }

        while (!_i2c.getControllerStatus().idle())
            ;

        return ErrorType::NoError;
    }

    // Write transaction, followed by a read transaction with restart in between
    ErrorType transfer(std::uint8_t addr, std::span<const std::byte> writebuf, std::span<std::byte> readbuf) noexcept
    {

        static constexpr unsigned int txFifoSize = 8;    // TODO hardcoded here?
        if (writebuf.size() > txFifoSize) {
            return ErrorType::NoError;    // TODO not implemented
        }

        while (!_i2c.getControllerStatus().idle())
            ;

        _i2c.setControllerTargetAddr(addr, I2c::Direction::Receive);
        _i2c.configureControllerOperation({.burstrun = true,
                                           .generateStart = true,
                                           .generateStop = true,
                                           .readOnTxEmpty = true,
                                           .transactionLength = readbuf.size()});

        readbuf = _i2c.readRxFifo(readbuf);

        while (!readbuf.empty()) {
            while (_i2c.getControllerRxFifoStatus().getCount() == 0) {
                if (_i2c.getControllerStatus().error()) {
                    DL_I2C_flushControllerTXFIFO(I2C0);
                    DL_I2C_flushControllerRXFIFO(I2C0);
                    // DL_I2C_disableControllerReadOnTXEmpty(I2C0);
                    return ErrorType::IoError;
                }
            }

            readbuf = _i2c.readRxFifo(readbuf);
        }

        // DL_I2C_disableControllerReadOnTXEmpty(I2C0);
        return ErrorType::NoError;
    }

  private:
    I2c& _i2c;
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_I2CCONTROLLER_HPP
