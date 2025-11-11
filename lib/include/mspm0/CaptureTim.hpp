#ifndef LIB_INCLUDE_MSPM0_CAPTURETIM_HPP
#define LIB_INCLUDE_MSPM0_CAPTURETIM_HPP

#include "Interrupt.hpp"
#include "RegSet.hpp"

#include "ti_msp_dl_config.h"

#include <chrono>
#include <expected>
#include <cstdint>

namespace mspm0 {

enum class CaptureTimError {
    NoError,
    NotSynced,
};

struct CaptureTimConfig {
    unsigned int irqLine;
    unsigned int channel;
    unsigned int prescaler;    // TODO max 0xff -> check somewhere?
};

template <CaptureTimConfig CFG_V>
class CaptureTim {
  public:
    using ErrorType = CaptureTimError;
    static constexpr auto intLine = std::integral_constant<unsigned int, TIMG8_INT_IRQn>{};

    static constexpr unsigned int timClk = 24'000'000;    // TODO hardcoded here

    constexpr CaptureTim(uintptr_t addr) noexcept
     : _pwrCtrl(addr)
     , _clkCtrl(addr)
     , _intCtrl(addr, detail::regSet::intRegOffset)
     , _commonRegs(new (reinterpret_cast<std::uint32_t*>(addr + commonRegOffset)) CommonRegisters)
     , _ctrRegs(new (reinterpret_cast<std::uint32_t*>(addr + ctrRegOffset)) CounterRegisters)
    { }

    void init() noexcept
    {
        _pwrCtrl.reset();
        _pwrCtrl.enable();

        _clkCtrl.setSource(detail::regSet::ClockControl::ClockSource::BusClk);
        _commonRegs->CPS = CFG_V.prescaler;

        _ctrRegs->LOAD = 0xffff;
        // static_assert((1 << CFG_V.resolution) - 1 == 0xffff);

        static constexpr std::uint32_t ctrctlmask =
            (0x3 << 28u) | (0x3 << 4u) | (0x1 << 1u) | (0x7 << 13u) | (0x7 << 10u) | (0x7 << 7u);
        _ctrRegs->CTRCTL &= ~ctrctlmask;
        _ctrRegs->CTRCTL |= (0x2 << 28u) | (0x2 << 4u) | (0x1 << 1u) | (CFG_V.channel << 13u) | (CFG_V.channel << 10u) |
                            (CFG_V.channel << 7u);    // set
                                                      // CM
        // to up,
        // REPEAT
        // to
        // continue,
        // CVAE
        // to
        // load
        // 0, select channel for CZC, CAC, CLC

        // automatic load must be disabled, because the load seems to happen before the captured value is transferred.
        // Therefore the capture register would always contain the load value (see ERRATA TIMER_ERR_01)
        _ctrRegs->CCCTL[CFG_V.channel] |= 0x20000 | (GPTIMER_CCCTL_01_ACOND_TIMCLK << 4u) |
                                          (GPTIMER_CCCTL_01_ZCOND_CC_TRIG_NO_EFFECT << 12u) |
                                          (GPTIMER_CCCTL_01_LCOND_CC_TRIG_NO_EFFECT << 8u) | 2;    // set
                                                                                                   // COC, capture
                                                                                                   // falling edge
        _commonRegs->CCPD &= ~(1 << CFG_V.channel);

        _intCtrl.enableInterrupts(DL_TIMERG_INTERRUPT_CC1_UP_EVENT | DL_TIMERG_INTERRUPT_OVERFLOW_EVENT);

        System::InterruptHandler::registerIsr(
            CFG_V.irqLine, System::InterruptHandler::CallbackType::create<CaptureTim, &CaptureTim::isr>(this));

        _commonRegs->CCLKCTL |= 1;
    }

    void enable() noexcept
    {
        // NVIC_EnableIRQ(static_cast<IRQn_Type>(CFG_V.irqLine));    // TODO remove cast
        NVIC_EnableIRQ(TIMG8_INT_IRQn);    // TODO use line nr in CFG_V
        // start counter:
        _ctrRegs->CTRCTL |= 1;    // TODO magic number
    }

    using PeriodType = std::chrono::duration<std::uint32_t, std::ratio<(CFG_V.prescaler + 1), timClk>>;

    std::expected<PeriodType, ErrorType> getPeriod() const noexcept
    {
        if (!_synced) {
            return std::unexpected(CaptureTimError::NotSynced);
        }

        return PeriodType{_ctrRegs->CC[CFG_V.channel]};
    }

    void isr() noexcept
    {
        switch (_intCtrl.getPending()) {
            case DL_TIMERG_IIDX_CC1_UP:
                _synced = true;
                _ctrRegs->CTR = 0;    // Manual reload, workaround for ERRATA TIMER_ERR_01
                // _callback(PeriodType{_ctrRegs->CC[CFG_V.channel]}); // TODO implement
                break;
            case DL_TIMERG_IIDX_OVERFLOW:
                /* If Timer reaches overflows then no PWM signal is detected and it
                 * requires re-synchronization
                 */
                _synced = false;
                break;
            default: break;
        }
    }

  private:
    struct CommonRegisters {
        std::uint32_t CCPD;
        std::uint32_t ODIS;
        std::uint32_t CCLKCTL;
        std::uint32_t CPS;
        std::uint32_t CPSV;
        std::uint32_t CTTTRIGCTRL;
        std::uint32_t reserved;
        std::uint32_t CTTTRIG;
    };

    struct CounterRegisters {
        std::uint32_t CTR;
        std::uint32_t CTRCTL;
        std::uint32_t LOAD;
        std::uint32_t reserved_0;
        std::uint32_t CC[6];
        std::uint32_t reserved_1[2];
        std::uint32_t CCCTL[6];
        std::uint32_t reserved_2[2];
        std::uint32_t OCTL[4];
        std::uint32_t reserved_3[4];
        std::uint32_t CCACT[4];
        std::uint32_t IFCTL[4];
    };

    detail::regSet::PowerControl _pwrCtrl;
    detail::regSet::ClockControl _clkCtrl;
    detail::regSet::InterruptControl _intCtrl;

    static constexpr uintptr_t commonRegOffset = 0x1100;
    static constexpr uintptr_t ctrRegOffset = 0x1800;

    volatile CommonRegisters* const _commonRegs;
    volatile CounterRegisters* const _ctrRegs;

    volatile bool _synced{};
    // fn_ref _callback;    // TODO implement
};

}    // namespace mspm0

#endif    // LIB_INCLUDE_MSPM0_CAPTURETIM_HPP
