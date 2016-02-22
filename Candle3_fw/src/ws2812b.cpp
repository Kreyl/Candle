/*
 * ws2812b.cpp
 *
 *  Created on: 05 апр. 2014 г.
 *      Author: Kreyl
 */

#include "ws2812b.h"
#include "cmd_uart_f10x.h"

LedWs_t LedWs;

// Tx timings
#define T0H_N       1
#define T1H_N       3
#define T_TOTAL_N   18

extern "C" {
// Wrapper for Tx Completed IRQ
void LedTxcIrq(void *p, uint32_t flags) {
    dmaStreamDisable(LED_DMA_STREAM);
    LedWs.IStopTx();
    //Uart.Printf("Irq\r");
}
static inline void LedTmrCallback(void *p) { LedWs.ITmrHandler(); }
} // "C"

void LedWs_t::Init() {
    // ==== Timer ====
    // Remap T15CH2 to PB15
//    bool AfioWasEnabled = (RCC->APB2ENR & RCC_APB2ENR_AFIOEN);
//    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;     // Enable AFIO
//    AFIO->MAPR2 |= AFIO_MAPR2_TIM15_REMAP;
//    if (!AfioWasEnabled) RCC->APB2ENR &= ~RCC_APB2ENR_AFIOEN;
    // Init tmr in PWM mode
    TxTmr.Init(LED_TMR);
    TxTmr.Enable();
    TxTmr.InitPwm(LED_GPIO, LED_PIN, LED_TMR_CHNL, invNotInverted, true);
    TxTmr.SetTopValue(T_TOTAL_N);
    TxTmr.SetPwm(0);
    TxTmr.EnableDmaOnUpdate();
    TxTmr.Disable();

    // ==== DMA ====
    dmaStreamAllocate     (LED_DMA_STREAM, IRQ_PRIO_LOW, LedTxcIrq, NULL);
    dmaStreamSetPeripheral(LED_DMA_STREAM, TxTmr.PCCR);
    dmaStreamSetMemory0   (LED_DMA_STREAM, BitBuf);
    dmaStreamSetMode      (LED_DMA_STREAM, LED_DMA_MODE);
}

void LedWs_t::SetCommonColor(Color_t Clr) {
    for(uint32_t i=0; i<LED_CNT; i++) IClr[i] = Clr;
    ISetCurrentColors();
}

void LedWs_t::SetCommonColorSmoothly(Color_t Clr, ClrSetupMode_t AMode) {
    chVTReset(&ITmr);
    IMode = AMode;
    for(uint32_t i=0; i<LED_CNT; i++) DesiredClr[i] = Clr;
    Indx = 0;   // Start with first LED
    ITmrHandler();
}

void LedWs_t::ITmrHandler() {
//    Uart.Printf("I1=%u\r", Indx);
    while(IClr[Indx] == DesiredClr[Indx]) {
        Indx++;
        if(Indx >= LED_CNT) return; // Setup completed
    }
    uint32_t Delay = ICalcDelayClr();
    // Adjust color(s) depending on mode
    if(IMode == csmOneByOne) IClr[Indx].Adjust(&DesiredClr[Indx]);
    else {  // Simultaneously
        for(uint8_t i=0; i<LED_CNT; i++) IClr[i].Adjust(&DesiredClr[Indx]);
    }
    ISetCurrentColors();
//    Uart.Printf("I=%u; D=%u\r", Indx, Delay);
    chVTSet(&ITmr, MS2ST(Delay), LedTmrCallback, NULL);
}

void LedWs_t::AppendBitsMadeOfByte(uint8_t Byte) {
    for(uint8_t i=0; i<8; i++) {
        if(Byte & 0x80) *PBit = T1H_N;
        else *PBit = T0H_N;
        PBit++;
        Byte <<= 1;
    }
}

void LedWs_t::ISetCurrentColors() {
    // Fill bit buffer
    PBit = &BitBuf[RST_BIT_CNT];
    for(uint32_t i=0; i<LED_CNT; i++) {
        AppendBitsMadeOfByte(IClr[i].Green);
        AppendBitsMadeOfByte(IClr[i].Red);
        AppendBitsMadeOfByte(IClr[i].Blue);
    }
    // Start transmission
    dmaStreamSetTransactionSize(LED_DMA_STREAM, TOTAL_BIT_CNT);
    dmaStreamEnable(LED_DMA_STREAM);
    TxTmr.SetCounter(0);

//    chSysLockFromIsr();
    TxTmr.Enable();
//    dmaWaitCompletion(LED_DMA_STREAM);
//    IStopTx();
//    chSysUnlockFromIsr();
}

uint32_t LedWs_t::ICalcDelayClr() {
    uint32_t DelayR = (IClr[Indx].Red   == DesiredClr[Indx].Red  )? 0 : ICalcDelay(IClr[Indx].Red);
    uint32_t DelayG = (IClr[Indx].Green == DesiredClr[Indx].Green)? 0 : ICalcDelay(IClr[Indx].Green);
    uint32_t DelayB = (IClr[Indx].Blue  == DesiredClr[Indx].Blue )? 0 : ICalcDelay(IClr[Indx].Blue);
//    Uart.Printf("I=%u; R=%u/%u; G=%u/%u; B=%u/%u\r",
//            Indx,
//            IClr[Indx].Red,   DesiredClr[Indx].Red,
//            IClr[Indx].Green, DesiredClr[Indx].Green,
//            IClr[Indx].Blue,  DesiredClr[Indx].Blue);
//    Uart.Printf("DR=%u; DG=%u; DB=%u\r", DelayR, DelayG, DelayB);
    uint32_t Rslt = DelayR;
    if(DelayG > Rslt) Rslt = DelayG;
    if(DelayB > Rslt) Rslt = DelayB;
    return Rslt;
}
