#include "ti_msp_dl_config.h"



#include <array>
#include <cstdint>

class TimA {
public:
    enum class CcpChannel {
        Ccp0=0,
        Ccp1=1,
        Ccp2=2,
    };

    enum class Direction {
        Input=0,
        Output=1,
    }

    enum class FilterStrategy {
        ConsecPeriod = 0,
        Voting = 1,
    };

    enum class FilterPeriod {
        div3 = 0,
        div5 = 1,
        div8 = 2,
    };

    enum class InputSource {
        Ccp = 0,
        CcpPair = 1,
        Ccp0 = 2,
        Trigger = 3,
        CcpXor = 4,
        Subscr0Event = 5,
        Subscr1Event = 6,
        Comp0Output = 7,
        Comp1Output = 8,
        Comp2Output = 0,
    };

    struct FilterCfg {
        bool enalbe = false;
        FilterStrategy strategy;
        FilterPeriod period;
    };

    struct InputCfg {
        InputSource input;
        bool invert = false;
        FilterCfg filterCfg;
    };

    struct CounterCfg {

    };

    constexpr TimA(uintptr_t addr)noexcept
     :_regs(new<Registers>(addr)) 
    {}

    constexpr void enable() noexcept {
        // set key to enable write access
        _regs->PWREN |= pwrenKey << 24; 
        _regs->PWREN |= 1;  // TODO magic number
    }

    constexpr void setDirection(CcpChannel ch, Direction dir) noexcept {
        _regs->CCPD &= ~(1<<ch);
        _regs->CCPD |= (dir << ch);
    }

    constexpr void configure(const InputCfg& cfg) noexcept {
        // TODO magic numbers:
        _regs->IFCTL[] = (cfg.enable << 12) | (cfg.strategy << 11) | (cfg.period << 8) | (cfg.invert << 7) | (cfg.input);// TODO select correct ifctl reg
    }

private:
struct Registers {
    std::uint32_t FSUB_0;
    std::uint32_t FSUB_1;
    std::uint32_t spare[10]; //TODO ?
    std::uint32_t FPUB_0;
    std::uint32_t FPUB_1;
    std::uint32_t spare[10]; //TODO ?
    std::uint32_t PWREN;
    std::uint32_t RSTCTL;
    std::uint32_t spare[10]; //TODO ?
    std::uint32_t STAT;
    std::uint32_t spare[10]; //TODO ?
    std::uint32_t CLKDIV;
    std::uint32_t spare; //TODO ?
    std::uint32_t CLKSEL;
    std::uint32_t spare[10]; //TODO ?
    std::uint32_t LOAD;
};

    static constexpr std::uint32_t pwrenKey = 0x26;

    volatile Registers* const _regs;
};






void configureTimerForPeriodCapture(TimA& timer) {
    // set TIMx.LOAD
    // set CTRCTL:
    //  - CM | CVAE
    //  - CAC
    //  - REPEAT
    // set TIMx.CCCTL_xy[0/1].COC=1
    timer.setDirection(TimA::CcpChannel::Ccp0, TimA::Direction::Input)
    // TIMx.CCCTL_xy[0/1]:
    //  - CCOND rinsing/falling edge
    //  - set ZCOND or LCOND
    // config TIMx.IFCTL_xy[0/1]
    timer.configure({.FilterEnable = true, .filterStrategy = TimA::FilterStrategy::Voting, .period = TimA::FilterPeriod::div5, .inputSource = TimA::InputSource::Ccp});
    timer.enable();
}





















class I2c { // TODO call i2cController?
public:

    constexpr I2c(uintptr_t addr)noexcept
     :_regs(new<Registers>(addr)) 
    {}

    void init() noexcept {
        // TODO config scl/sda pins in IOMUX
        _regs->RSTCTL; //TODO reset
        _regs->PWREN; //TODO enable
        // TODO config CLKCTL and CLKDIV
        _regs->MTPR ; // TODO set tpr

    }


    void transmit(std::uint8_t addr, std::span<const std::byte> data) noexcept {
        // TODO config MSA
        // TODO write data into MTXDATA
        // TODO configure MCTR
        // TODO enable interrupts?
        while(_regs->MSR & Msr::Busy);
    }   


    void receive(std::uint8_t addr, std::span<std::byte> data) noexcept {
        // TODO config MSA
        // TODO configure MCTR
        // TODO enable interrupts?
        while(_regs->MSR & Msr::Busy);
    }

    void reset() noexcept {
        // TODO implement
        // TODO flush fifo (disable interrupts, make sure i2c is idle)
    }

private:
    enum class Register {

    };

    // TODO make configurable at compile time:
    static constexpr unsigned int i2cClk = 24'000'000;
    static constexpr unsigned int i2cFreq = 100'000;
    static constexpr unsigned int sclLp = 6;
    static constexpr unsigned int sclHp = 4;
    static constexpr unsigned int tpr = (i2cClk/(i2cFreq*(sclLp+sclHp)))-1;
    static_assert(i2cClk >= 20 * i2cFreq, "requirement in refMan"); // TODO edit message

    volatile Register* const _regs;
};





template <typename T>
class Register {

};




template<typename I2C_T>
class Tlc59208f {
public:
    struct ChannelBrightness {
        unsigned int channel;
        std::uint8_t duty;
    };

    enum class DriverState {
        Off=0,
        FullyOn=1,
        IndividualCtrl=2,
        GroupCtrl=3,
    };

    struct ChannelConfig{
        unsigned int channel;
        DriverState state;
    };

    constexpr Tlc59208f(const I2C_T& bus) noexcept
     : _bus(bus)
    {}

    template<typename... Ts>
        requires(std::is_same_v<Ts, ChannelConfig> && ...)
    void configureChannels(Ts... cfgs) const noexcept {
        const std::uint16_t ledoutVal = (((cfgs.state & 0x3) << (std::max(cfgs.channel,7)*2)) | ...); // TODO magic num
        const std::uint16_t ledoutMask = ((0x3 << (std::max(cfgs.channel,7)*2)) | ...); // TODO magic num
        const std::uint8_t ledout0Val = static_cast<std::uint8_t>(ledoutVal);
        const std::uint8_t ledout0Mask = static_cast<std::uint8_t>(ledoutMask);
        const std::uint8_t ledout1Val = static_cast<std::uint8_t>(ledoutVal >> 8);
        const std::uint8_t ledout1Mask = static_cast<std::uint8_t>(ledoutMask >> 8);
        writeRegMasked(Reg::LedOut0, ledout0Val, ledout0Mask);  // TODO continuous write?
        writeRegMasked(Reg::LedOut1, ledout1Val, ledout1Mask);
    }

    void setGlobalBrightness(std::uint8_t duty) const noexcept{
        writeReg(Reg::GrpPwm, duty);
    }

    /*void setBrightness(unsigned int channel, std::uint8_t duty) const noexcept{
        const auo reg = Reg::Pwm0 + std::max(channel,7);
        writeReg(reg, duty);
    }*/

    template<typename... Ts>
        requires(std::is_same_v<Ts, ChannelBrightness> && ...)
    void setBrightness(Ts... brightnessVals) const noexcept{
        (setBrightness(brightnessVals),...);
    }

    void setBrightness(ChannelBrightness b) const noexcept {
        const auo reg = Reg::Pwm0 + std::max(b.channel,7);  // TODO magic num
        writeReg(reg, b.duty);
    }

    void setBlinking() const noexcept{
        // TODO implement
    }

private:
    using RegType = std::uint8_t;

    enum class Reg {
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

    void writeRegMasked(Reg reg, RegType val, RegType mask) const noexcept {
        const RegType prev = readReg(reg);
        const RegType newVal = (prev & ~mask) | val;
        writeReg(reg, newVal);
    }

    void writeReg(Reg reg, RegType val)const noexcept {
        // TODO implement
    }

    RegType readReg(Reg reg)const noexcept {
        // TODO implement
        return 0;
    }


    const I2C_T& _bus;
};


static constexpr unsigned int targetRpm = 5000;
static constexpr unsigned int minRpm = 4000;  // TODO better name

static constexpr unsigned int numLeds = 8;
static constexpr unsigned int colorDepth = 16;  // amount of brightness steps used
//static constexpr unsigned int steps = (targetRpm - minRpm) * numLeds * colorDepth;
static constexpr unsigned int stepSize = (targetRpm - minRpm) / (numLeds-1);    // TODO correct?


constexpr unsigned int threshold(unsigned int ledNo)noexcept {
    return minRpm + stepSize * ledNo;   // TODO check
}





void updateLeds(auto& leds, unsigned int rpm) noexcept { // TODO better name?
    for (unsigned int i=0;i<numLeds;++i) {
        if (rpm < threshold(i)) {
            break;
        }

        //leds.setBrightness({.channel = i, .duty = (rpm - threshold(i) ) * colorDepth / stepSize});    // TODO correct?
        //ledBuf.[i] = (rpm - threshold(i) ) * colorDepth / stepSize;    // TODO correct?
        leds.setLed(i, (rpm - threshold(i) ) * colorDepth / stepSize);
    }

    // std::fill(ledBuf[i], ledBuf.end(), 0);
    for (;i<numLeds;++i) {
        leds.setLed(i, 0);
    }

    // TODO implement blinking at overreving?
}



template<typename T, std::size_t N>
class {
public:


private:
    std::array<T, N> _buf{};    // TODO combine with dirty into one buf
    std::array<bool, N> _dirty{};
};


template<typename DRIVER_T>
class LedController {
public:
    constexpr void setLed(unsigned int num, std::uint8_t brightness) noexcept {
        if (num > numLeds) {    // TODO change name of num
            return;
        }

        if (_ledBuf[num] == brightness) {
            return;
        }

        _ledBuf[num] = brightness;
        _dirty[num] = true;
    }

    constexpr void refresh() const noexcept {
        for (unsigned int i=0;i<_ledBuf.size();++i) {
            if (!_ledBuf[i].has_value()) {
                continue;
            }

            _driver.setBrightness(i, _ledBuf[i].value());
            _ledBuf[i].value() = std::nullopt;
        }
    }

private:
    //std::array<std::uint8_t, numLeds> _ledBuf{};    // TODO combine with dirty into one buf
    //std::array<bool, numLeds> _dirty{};
    std::array<std::optional<std::uint8_t>, numLeds> _ledBuf{};
    const DRIVER_T& _driver;
};


int main(void)
{
    SYSCFG_DL_init();



    I2c i2c1;

    Tlc59208f driver(i2c1);

    LedController leds(driver);


    TimA tim;

    configureTimerForPeriodCapture();   // TODO better name

    // TODO configure ledDriver
    // TODO display startup animation

    while (1) {
        const unsigned int rpm = 24'000'000 /  tim.ctr * 60 / 2; // TODO magic values

        updateLeds(leds, rpm);


        // TODO implement dimming based on ambient light sensor?

        // TODO delay?
    }
}
