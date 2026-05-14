/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the LP_MSPM0C1104
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

#include "Devices.hpp"

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);

    DL_GPIO_enablePower(GPIOA);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{
    // DL_SYSCTL_disableNRSTPin();
    Devices::sysCtl.disableNrstPin();

    // DL_GPIO_initPeripheralInputFunctionFeatures(IOMUX_PINCM1, IOMUX_PINCM1_PF_I2C0_SDA, DL_GPIO_INVERSION_DISABLE,
    //                                             DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
    //                                             DL_GPIO_WAKEUP_DISABLE);

    // Devices::ioMux.getPin(IOMUX_PINCM1).configure({.function = IOMUX_PINCM1_PF_I2C0_SDA, .openDrain = true});
    // Devices::ioMux.getPin(IOMUX_PINCM2).configure({.function = IOMUX_PINCM2_PF_I2C0_SCL, .openDrain = true});

    Devices::pin1.configure(
        {.function = mspm0::IoMux::Pin1Functions::I2c0_Sda, .connected = true, .inputEnable = true, .openDrain = true});
    Devices::pin2.configure(
        {.function = mspm0::IoMux::Pin2Functions::I2c0_Scl, .connected = true, .inputEnable = true, .openDrain = true});

    // DL_GPIO_initPeripheralInputFunctionFeatures(IOMUX_PINCM2, IOMUX_PINCM2_PF_I2C0_SCL, DL_GPIO_INVERSION_DISABLE,
    //                                             DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
    //                                             DL_GPIO_WAKEUP_DISABLE);

    // DL_GPIO_enableHiZ(IOMUX_PINCM1);
    // DL_GPIO_enableHiZ(IOMUX_PINCM2);

    // DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM28, IOMUX_PINCM28_PF_TIMG8_CCP1);
    Devices::pin28.configure(
        {.function = mspm0::IoMux::Pin28Functions::TimG8_Ccp1, .connected = true, .inputEnable = true});
}

SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{
    // DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    Devices::sysCtl.configureSysOsc({.freq = mspm0::SysControl::SysOscFreq::Base32Mhz});
    // DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);
    Devices::sysCtl.configureMclk({.divider = 0});

    // Low Power Mode is configured to be SLEEP0
    // DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
}
