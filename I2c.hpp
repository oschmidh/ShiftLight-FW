#ifndef I2C_HPP
#define I2C_HPP

#include <ti/driverlib/dl_i2c.h>
#include <ti/driverlib/driverlib.h>

#include <span>
#include <cstdint>

enum class I2cError {
    NoError,
    IoError,
    InvalidParam,
};

class I2c {    // TODO call i2cController?
  public:
    using ErrorType = I2cError;

    I2c() noexcept { }

    void init() noexcept
    {
        DL_I2C_reset(I2C0);
        DL_I2C_enablePower(I2C0);

        constexpr DL_I2C_ClockConfig clkCfg{.clockSel = DL_I2C_CLOCK_BUSCLK, .divideRatio = DL_I2C_CLOCK_DIVIDE_1};
        DL_I2C_setClockConfig(I2C0, &clkCfg);
        DL_I2C_disableAnalogGlitchFilter(I2C0);    // TODO needed?

        /* Configure Controller Mode */
        DL_I2C_resetControllerTransfer(I2C0);

        // TODO make configurable at compile time:
        static constexpr unsigned int i2cClk = 24'000'000;
        static constexpr unsigned int i2cFreq = 100'000;
        static_assert(i2cClk >= 20 * i2cFreq, "requirement in refMan");    // TODO edit message
        static constexpr unsigned int sclLp = 6;
        static constexpr unsigned int sclHp = 4;
        static constexpr unsigned int tpr = (i2cClk / (i2cFreq * (sclLp + sclHp))) - 1;
        static_assert(tpr <= 0x7F);

        DL_I2C_setTimerPeriod(I2C0, tpr);
        DL_I2C_setControllerTXFIFOThreshold(I2C0, DL_I2C_TX_FIFO_LEVEL_BYTES_1);
        DL_I2C_setControllerRXFIFOThreshold(I2C0, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
        DL_I2C_enableControllerClockStretching(I2C0);

        DL_I2C_enableController(I2C0);
    }

    // TODO should be std::byte instead of uint8?
    ErrorType write(std::uint8_t addr, std::span<const std::uint8_t> data) const noexcept
    {
        const auto size = data.size();
        if (size > 0xfff) {                    // TODO unlikely?
            return ErrorType::InvalidParam;    // TODO return error
        }

        while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE))
            ;

        const auto amount = DL_I2C_fillControllerTXFIFO(I2C0, data.data(), data.size());
        data = data.subspan(amount);    // TODO is it allowed if offset == size?

        DL_I2C_startControllerTransfer(I2C0, addr, DL_I2C_CONTROLLER_DIRECTION_TX, size);

        while (!data.empty()) {
            const auto amount = DL_I2C_fillControllerTXFIFO(I2C0, data.data(), data.size());
            data = data.subspan(amount);    // TODO is it allowed if offset == size?
        }

        /* Poll until the Controller writes all bytes */
        while (DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
            ;

        /* Trap if there was an error */
        if (DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_ERROR) {
            DL_I2C_flushControllerTXFIFO(I2C0);
            return ErrorType::IoError;    // TODO return error code?
        }

        while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE))
            ;

        return ErrorType::NoError;
    }

    // Write transaction, followed by a read transaction with restart in between
    ErrorType transfer(std::uint8_t addr, std::span<const std::uint8_t> writebuf,
                       std::span<std::uint8_t> readbuf) const noexcept
    {    // TODO should be std::byte instead of uint8?

        static constexpr unsigned int txFifoSize = 8;    // TODO hardcoded here?
        if (writebuf.size() > txFifoSize) {
            return ErrorType::NoError;    // TODO not implemented
        }

        while (!(DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_IDLE))
            ;

        DL_I2C_enableControllerReadOnTXEmpty(I2C0);
        DL_I2C_startControllerTransfer(I2C0, addr, DL_I2C_CONTROLLER_DIRECTION_RX, readbuf.size());

        for (auto& byte : readbuf) {
            while (DL_I2C_isControllerRXFIFOEmpty(I2C0)) {
                if (DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_ERROR) {
                    DL_I2C_flushControllerTXFIFO(I2C0);
                    DL_I2C_flushControllerRXFIFO(I2C0);
                    DL_I2C_disableControllerReadOnTXEmpty(I2C0);
                    return ErrorType::IoError;    // TODO return error code?
                }
            }

            byte = DL_I2C_receiveControllerData(I2C0);
        }

        DL_I2C_disableControllerReadOnTXEmpty(I2C0);
        return ErrorType::NoError;
    }

  private:
};

#endif
