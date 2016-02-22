/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    STM32F1xx/hal_lld.c
 * @brief   STM32F1xx HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "clocking_f100.h"

/**
 * @brief   Initializes the backup domain.
 * @note    WARNING! Changing clock source impossible without resetting
 *          of the whole BKP domain.
 */
//static void hal_lld_backup_domain_init(void) {
//
//  /* Backup domain access enabled and left open.*/
//  PWR->CR |= PWR_CR_DBP;
//
//#if HAL_USE_RTC
//  /* Reset BKP domain if different clock source selected.*/
//  if ((RCC->BDCR & STM32_RTCSEL_MASK) != STM32_RTCSEL){
//    /* Backup domain reset.*/
//    RCC->BDCR = RCC_BDCR_BDRST;
//    RCC->BDCR = 0;
//  }
//
//  /* If enabled then the LSE is started.*/
//#if STM32_LSE_ENABLED
//  RCC->BDCR |= RCC_BDCR_LSEON;
//  while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0)
//    ;                                     /* Waits until LSE is stable.   */
//#endif /* STM32_LSE_ENABLED */
//
//#if STM32_RTCSEL != STM32_RTCSEL_NOCLOCK
//  /* If the backup domain hasn't been initialized yet then proceed with
//     initialization.*/
//  if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0) {
//    /* Selects clock source.*/
//    RCC->BDCR |= STM32_RTCSEL;
//
//    /* Prescaler value loaded in registers.*/
//    rtc_lld_set_prescaler();
//
//    /* RTC clock enabled.*/
//    RCC->BDCR |= RCC_BDCR_RTCEN;
//  }
//#endif /* STM32_RTCSEL != STM32_RTCSEL_NOCLOCK */
//#endif /* HAL_USE_RTC */
//}

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {

  /* Reset of all peripherals.*/
  rccResetAPB1(0xFFFFFFFF);
  rccResetAPB2(0xFFFFFFFF);

  /* SysTick initialization using the system clock.*/
  SysTick->LOAD = Clk.AHBFreqHz / CH_FREQUENCY - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_ENABLE_Msk |
                  SysTick_CTRL_TICKINT_Msk;

  /* DWT cycle counter enable.*/
//  SCS_DEMCR |= SCS_DEMCR_TRCENA;
//  DWT_CTRL  |= DWT_CTRL_CYCCNTENA;

  /* PWR and BD clocks enabled.*/
  rccEnablePWRInterface(FALSE);
//  rccEnableBKPInterface(FALSE);

  /* Initializes the backup domain.*/
//  hal_lld_backup_domain_init();

#if defined(STM32_DMA_REQUIRED)
  dmaInit();
#endif

  /* Programmable voltage detector enable.*/
#if STM32_PVD_ENABLE
  PWR->CR |= PWR_CR_PVDE | (STM32_PLS & STM32_PLS_MASK);
#endif /* STM32_PVD_ENABLE */
}
