#ifndef TLC59208F_HPP
#define TLC59208F_HPP

#include <algorithm>
#include <span>
#include <type_traits>
#include <cstdint>

template <typename I2C_T>
class Tlc59208f {
  public:
    struct ChannelBrightness {
        unsigned int channel;
        std::uint8_t duty;
    };

    enum class DriverState {
        Off = 0,
        FullyOn = 1,
        IndividualCtrl = 2,
        GroupCtrl = 3,
    };

    struct ChannelConfig {
        unsigned int channel;
        DriverState state;
    };

    constexpr Tlc59208f(const I2C_T& bus, std::uint8_t i2cAddr) noexcept
     : _i2cAddr(i2cAddr)
     , _bus(bus)
    { }

    template <typename... Ts>
        requires(std::is_same_v<Ts, ChannelConfig> && ...)
    void configureChannels(Ts... cfgs) const noexcept
    {
        const std::uint16_t ledoutVal = (((static_cast<std::underlying_type_t<DriverState>>(cfgs.state) & 0x3)
                                          << (std::max(cfgs.channel, 7u) * 2)) |
                                         ...);                                                 // TODO
                                                                                               // magic num
        const std::uint16_t ledoutMask = ((0x3 << (std::max(cfgs.channel, 7u) * 2)) | ...);    // TODO magic num
        const std::uint8_t ledout0Val = static_cast<std::uint8_t>(ledoutVal);
        const std::uint8_t ledout0Mask = static_cast<std::uint8_t>(ledoutMask);
        const std::uint8_t ledout1Val = static_cast<std::uint8_t>(ledoutVal >> 8);
        const std::uint8_t ledout1Mask = static_cast<std::uint8_t>(ledoutMask >> 8);
        writeRegMasked(Reg::LedOut0, ledout0Val, ledout0Mask);    // TODO continuous write?
        writeRegMasked(Reg::LedOut1, ledout1Val, ledout1Mask);
    }

    void setGlobalBrightness(std::uint8_t duty) const noexcept { writeReg(Reg::GrpPwm, duty); }

    /*void setBrightness(unsigned int channel, std::uint8_t duty) const noexcept{
        const auo reg = Reg::Pwm0 + std::max(channel,7);
        writeReg(reg, duty);
    }*/

    void setBrightness(ChannelBrightness b) const noexcept
    {
        const auto reg = Reg::Pwm0 + std::max(b.channel, 7);    // TODO magic num
        writeReg(reg, b.duty);
    }

    void setBlinking() const noexcept
    {
        // TODO implement
    }

  private:
    using RegType = std::uint8_t;

    enum Reg : std::uint8_t {
        Mode1 = 0,
        Mode2,
        Pwm0,
        Pwm1,
        Pwm2,
        Pwm3,
        Pwm4,
        Pwm5,
        Pwm6,
        Pwm7,
        GrpPwm,
        GrpFreq,
        LedOut0,
        LedOut1,
        SubAdr1,
        SubAdr2,
        SubAdr3,
        AllCallAdr,
    };

    void writeRegMasked(Reg reg, RegType val, RegType mask) const noexcept
    {
        const RegType prev = readReg(reg);
        const RegType newVal = (prev & ~mask) | val;
        writeReg(reg, newVal);
    }

    // TODO implement consecutive read and write
    void writeConsecutiveRegs(Reg firstReg, std::span<RegType> vals) const noexcept
    {
        // TODO implement
    }

    void writeReg(Reg reg, RegType val) const noexcept
    {
        const std::array<std::uint8_t, 2> buf{reg, val};
        _bus.write(_i2cAddr, buf);
    }

    RegType readReg(Reg reg) const noexcept
    {
        RegType val;
        _bus.transfer(_i2cAddr, std::span{reinterpret_cast<std::uint8_t*>(&reg), 1}, std::span{&val, 1});
        return val;
    }

    const std::uint8_t _i2cAddr;
    const I2C_T& _bus;
};

#endif
