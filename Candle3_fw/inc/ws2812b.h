/*
 * ws2812b.h
 *
 *  Created on: 05 апр. 2014 г.
 *      Author: Kreyl
 */

#ifndef WS2812B_H_
#define WS2812B_H_

#include "stm32f10x.h"
#include "ch.h"
#include "hal.h"
#include "kl_lib_f100.h"
#include "color.h"

#define LED_CNT         1
#define RST_BIT_CNT     45 // 45 zero bits to produce reset
#define DATA_BIT_CNT    (LED_CNT * 3 * 8)   // 3 channels 8 bit each
#define TOTAL_BIT_CNT   (DATA_BIT_CNT + RST_BIT_CNT)

#define LED_TMR         TIM1
#define LED_TMR_CHNL    1
#define LED_GPIO        GPIOA
#define LED_PIN         8

// DMA
#define LED_DMA_STREAM  STM32_DMA1_STREAM5  // TIM1 UP
#define LED_DMA_MODE    DMA_PRIORITY_VERYHIGH \
                        | STM32_DMA_CR_MSIZE_BYTE \
                        | STM32_DMA_CR_PSIZE_HWORD \
                        | STM32_DMA_CR_MINC     /* Memory pointer increase */ \
                        | STM32_DMA_CR_DIR_M2P  /* Direction is memory to peripheral */ \
                        | STM32_DMA_CR_TCIE     /* Enable Transmission Complete IRQ */


enum ClrSetupMode_t {csmOneByOne, csmSimultaneously};

class LedWs_t {
private:
    Timer_t TxTmr;
    VirtualTimer ITmr;
    uint8_t BitBuf[TOTAL_BIT_CNT], *PBit, Indx;
    Color_t IClr[LED_CNT];
    ClrSetupMode_t IMode;
    void AppendBitsMadeOfByte(uint8_t Byte);
    void ISetCurrentColors();
    uint32_t ICalcDelay(uint16_t AValue) { return (uint32_t)((810 / (AValue+4)) + 1); }
    uint32_t ICalcDelayClr();
public:
    Color_t DesiredClr[LED_CNT];
    void Init();
    void SetCommonColor(Color_t Clr);
    void SetCommonColorSmoothly(Color_t Clr, ClrSetupMode_t AMode);
    // Inner use
    void IStopTx() { TxTmr.SetPwm(0); TxTmr.Disable(); }
    void ITmrHandler();
};

extern LedWs_t LedWs;

#endif /* WS2812B_H_ */
