#ifndef LIB_INCLUDE_DRIVERS_TLC59208F_HPP
#define LIB_INCLUDE_DRIVERS_TLC59208F_HPP

#include <tmp/Registers.hpp>
#include <tmp/RegisterOperations.hpp>

#include <algorithm>
#include <expected>
#include <span>
#include <type_traits>
#include <cstdint>

template <typename CONTENT_T>
struct Value {
    tmp::get_datatype_t<CONTENT_T> value;
};

template <typename ACCESS_POLICY_T, tmp::concepts::content... CONTENT_Ts>
constexpr void set(ACCESS_POLICY_T& accessor, Value<CONTENT_Ts>... values) noexcept
{    // TODO get return type from access policy
    using ContentList = tmp::content_list<CONTENT_Ts...>;
    using RegList = tmp::create_register_list_t<ContentList>;
    using RegType = tmp::Registers<RegList>;
    using AddrList = tmp::address_list_t<ContentList>;

    std::array<typename RegType::value_type, tmp::count_v<AddrList>> raw{};

    // static constexpr auto addresses = tmp::address_array<AddrList>::value;

    [&accessor, &raw]<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
        (accessor.read(tmp::address_v<tmp::at_t<IDX_Vs, AddrList>>, raw[IDX_Vs]), ...);
    }(std::make_index_sequence<raw.size()>());

    RegType regs{raw};
    // (regs.set(contents), ...);
    // (tmp::set<CONTENT_Ts, RegList>(values.value, regs), ...);
    (tmp::set<CONTENT_Ts, RegList>(values.value, raw.data()), ...);

    [&accessor, &raw]<std::size_t... IDX_Vs>(std::index_sequence<IDX_Vs...>) noexcept {
        (accessor.write(tmp::address_v<tmp::at_t<IDX_Vs, AddrList>>, raw[IDX_Vs]), ...);
    }(std::make_index_sequence<raw.size()>());
}

template <typename ACCESS_POLICY_T, tmp::concepts::content CONTENT_T>
    requires(tmp::get_mask_v<CONTENT_T> == tmp::get_mask_v<tmp::get_register_t<CONTENT_T>>)
constexpr void set(ACCESS_POLICY_T& accessor, Value<CONTENT_T> values) noexcept
{    // TODO get return type from access policy
    accessor.write(tmp::get_address_v<CONTENT_T>, values.value);
}

template <typename I2C_T>
class I2cWrapper {
  public:
    constexpr I2cWrapper(const I2C_T& bus, std::uint8_t i2cAddr) noexcept
     : _addr(i2cAddr)
     , _bus(bus)
    { }

    void read(std::uint8_t regAddr, std::uint8_t& val) const noexcept
    {
        _bus.transfer(_addr, std::span{reinterpret_cast<std::uint8_t*>(&regAddr), 1}, std::span{&val, 1});
    }

    void write(std::uint8_t regAddr, std::uint8_t val) noexcept
    {
        const std::array<std::uint8_t, 2> buf{regAddr, val};
        _bus.write(_addr, buf);
    }

  private:
    const std::uint8_t _addr;
    const I2C_T& _bus;
};

template <typename I2C_T>
class Tlc59208f {
  public:
    enum class DriverState {
        Off = 0,
        FullyOn = 1,
        IndividualCtrl = 2,
        GroupCtrl = 3,
    };

  private:
    using Mode1 = tmp::reg<tmp::address<std::uint8_t, 0x1>, tmp::reset_value<0x11>, tmp::datatype<std::uint8_t>>;
    using Mode2 = tmp::reg<tmp::address<std::uint8_t, 0x2>, tmp::reset_value<0x03>, tmp::datatype<std::uint8_t>>;
    using GrpPwm = tmp::reg<tmp::address<std::uint8_t, 0xA>, tmp::reset_value<0>, tmp::datatype<std::uint8_t>>;
    using LedOut0 = tmp::reg<tmp::address<std::uint8_t, 0xC>, tmp::reset_value<0>, tmp::datatype<std::uint8_t>>;
    using LedOut1 = tmp::reg<tmp::address<std::uint8_t, 0xD>, tmp::reset_value<0>, tmp::datatype<std::uint8_t>>;

    using Sleep = tmp::content<Mode1, tmp::offset<4>, tmp::width<1>>;
    using GroupDutyCycle = tmp::content<GrpPwm>;
    using Led0OutpState = tmp::content<LedOut0, tmp::offset<0>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led1OutpState = tmp::content<LedOut0, tmp::offset<2>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led2OutpState = tmp::content<LedOut0, tmp::offset<4>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led3OutpState = tmp::content<LedOut0, tmp::offset<6>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led4OutpState = tmp::content<LedOut1, tmp::offset<0>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led5OutpState = tmp::content<LedOut1, tmp::offset<2>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led6OutpState = tmp::content<LedOut1, tmp::offset<4>, tmp::width<2>, tmp::typelist<>, DriverState>;
    using Led7OutpState = tmp::content<LedOut1, tmp::offset<6>, tmp::width<2>, tmp::typelist<>, DriverState>;

  public:
    using ErrorType = I2C_T::ErrorType;
    using BrightnessType = std::uint8_t;

    struct ChannelBrightness {
        unsigned int channel;
        BrightnessType duty;
    };

    struct ChannelConfig {
        unsigned int channel;
        DriverState state;
    };

    struct DevConfig {
        bool sleep;
    };

    constexpr Tlc59208f(const I2C_T& bus, std::uint8_t i2cAddr) noexcept
     : _bus(bus, i2cAddr)
    { }

    ErrorType configure(DevConfig cfg) noexcept
    {
        // return writeReg(Reg::Mode1, static_cast<std::underlying_type_t<Mode>>(cfg.mode));
        // return set(_bus, Value<Sleep>{cfg.mode == Mode::Sleep});
        set(_bus, Value<Sleep>{cfg.sleep});
        return {};
    }

    template <typename... Ts>
        requires(std::is_same_v<Ts, ChannelConfig> && ...)
    ErrorType configureChannels(Ts... cfgs) noexcept
    {
        const std::uint16_t ledoutVal = (((static_cast<std::underlying_type_t<DriverState>>(cfgs.state) & 0x3)
                                          << (std::min(cfgs.channel, 7u) * 2u)) |
                                         ...);                                                  // TODO
                                                                                                // magic num
        const std::uint16_t ledoutMask = ((0x3 << (std::min(cfgs.channel, 7u) * 2u)) | ...);    // TODO magic num
        const std::uint8_t ledout0Val = static_cast<std::uint8_t>(ledoutVal);
        const std::uint8_t ledout0Mask = static_cast<std::uint8_t>(ledoutMask);
        const std::uint8_t ledout1Val = static_cast<std::uint8_t>(ledoutVal >> 8u);
        const std::uint8_t ledout1Mask = static_cast<std::uint8_t>(ledoutMask >> 8u);
        if (const auto err = writeRegMasked(Reg::LedOut0, ledout0Val, ledout0Mask); err != ErrorType::NoError) {
            return err;
        }    // TODO continuous write?
        return writeRegMasked(Reg::LedOut1, ledout1Val, ledout1Mask);
    }

    ErrorType setGlobalBrightness(BrightnessType duty) noexcept
    {
        // return writeReg(Reg::GrpPwm, duty);
        set(_bus, Value<GroupDutyCycle>{duty});
        return {};
    }

    /*void setBrightness(unsigned int channel, std::uint8_t duty) const noexcept{
        const auo reg = Reg::Pwm0 + std::min(channel,7);
        writeReg(reg, duty);
    }*/

    ErrorType setBrightness(ChannelBrightness b) noexcept
    {
        const auto reg = static_cast<Reg>(Reg::Pwm0 + std::min(b.channel, 7u));    // TODO magic num
        return writeReg(reg, b.duty);
    }

    void setBlinking() const noexcept
    {
        // TODO implement
    }

  private:
    using RegType = std::uint8_t;

    enum Reg : std::uint8_t {
        Mode1_ = 0,
        Mode2_,
        Pwm0,
        Pwm1,
        Pwm2,
        Pwm3,
        Pwm4,
        Pwm5,
        Pwm6,
        Pwm7,
        GrpPwm_,
        GrpFreq,
        LedOut0_,
        LedOut1_,
        SubAdr1,
        SubAdr2,
        SubAdr3,
        AllCallAdr,
    };

    ErrorType writeRegMasked(Reg reg, RegType val, RegType mask) noexcept
    {
        const auto prev = readReg(reg);
        if (!prev.has_value()) {
            return prev.error();
        }
        const RegType newVal = (prev.value() & ~mask) | val;
        return writeReg(reg, newVal);
    }

    // TODO implement consecutive read and write
    ErrorType writeConsecutiveRegs(Reg firstReg, std::span<RegType> vals) const noexcept
    {
        // TODO implement
    }

    ErrorType writeReg(Reg reg, RegType val) noexcept
    {
        _bus.write(reg, val);
        return {};
    }

    std::expected<RegType, ErrorType> readReg(Reg reg) const noexcept
    {
        RegType val;
        _bus.read(static_cast<std::uint8_t>(reg), val);
        return val;
    }

    I2cWrapper<I2C_T> _bus;
};

#endif    // LIB_INCLUDE_DRIVERS_TLC59208F_HPP
