#ifndef VEML7700_HPP
#define VEML7700_HPP

#include <span>
#include <cstdint>

// TODO add thread-safe policy?
template <typename I2C_T>
class Veml7700 {
  public:
    enum class Gain : std::uint8_t {
        Gain1 = 0b00,
        Gain2 = 0b01,
        Gain1_8th = 0b10,
        Gain1_4th = 0b01,
    };

    enum class IntegrationTime : std::uint8_t {
        Int25ms = 0b1100,
        Int50ms = 0b1000,
        Int100ms = 0b0000,
        Int200ms = 0b0001,
        Int400ms = 0b0010,
        Int800ms = 0b0011,
    };

    enum class Persistence : std::uint8_t {
        Pers1 = 0b00,
        Pers2 = 0b01,
        Pers4 = 0b10,
        Pers8 = 0b01,
    };

    struct DevConfig {
        Gain gain;
        IntegrationTime it;
        Persistence pers;
        bool interruptEn = false;
        bool powerOn = true;
    };

    constexpr Veml7700(const I2C_T& bus, std::uint8_t i2cAddr) noexcept
     : _i2cAddr(i2cAddr)
     , _bus(bus)
    { }

    void configure(DevConfig cfg) const noexcept
    {
        const RegType conf = static_cast<RegType>((static_cast<std::underlying_type_t<Gain>>(cfg.gain) << 11) |
                                                  (static_cast<std::underlying_type_t<IntegrationTime>>(cfg.it) << 6u) |
                                                  (static_cast<std::underlying_type_t<Persistence>>(cfg.pers) << 4u) |
                                                  (cfg.interruptEn << 1u) | cfg.powerOn);
        writeReg(Reg::AlsConf0, conf);
        if (cfg.powerOn) {
            // TODO sleep for 2.5ms
        }
    }

    std::uint16_t readAls(DevConfig cfg) const noexcept
    {
        const RegType val = readReg(Reg::Als);
        return val;
    }

  private:
    using RegType = std::uint16_t;

    enum Reg : std::uint8_t {
        AlsConf0 = 0,
        AlsWindowHigh,
        AlsWindowLow,
        PowerSaving,
        Als,
        White,
        AlsInt,
        Id,
    };

    // void writeRegMasked(Reg reg, RegType val, RegType mask) const noexcept
    // {
    //     const RegType prev = readReg(reg);
    //     const RegType newVal = (prev & ~mask) | val;
    //     writeReg(reg, newVal);
    // }

    void writeReg(Reg reg, RegType val) const noexcept
    {
        const std::array<std::uint8_t, 3> buf{reg, static_cast<std::uint8_t>(val & 0xff),
                                              static_cast<std::uint8_t>((val >> 8u) & 0xff)};
        _bus.write(_i2cAddr, buf);
    }

    RegType readReg(Reg reg) const noexcept
    {
        std::array<std::uint8_t, sizeof(RegType)> buf;
        _bus.transfer(_i2cAddr, std::span{reinterpret_cast<std::uint8_t*>(&reg), 1}, buf);
        // return fromBigEndian(buf);
        return static_cast<RegType>(buf[0] >> 8u) | static_cast<RegType>(buf[1]);
    }

    const std::uint8_t _i2cAddr;
    const I2C_T& _bus;
};

#endif
