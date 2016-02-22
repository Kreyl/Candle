/*
 * led.cpp
 *
 *  Created on: 03.11.2011
 *      Author: g.kruglov
 */

#include "led.h"

// =============================== LedBlink_t ==================================
//void LedBlink_t::Blink(uint32_t ABlinkDelay) {
//    IsInsideBlink = true;
//    IBlinkDelay = ABlinkDelay;
//    On();
//    Delay.Reset(&ITimer);
//}
//
//void LedBlink_t::Task() {
//    if (!IsInsideBlink) return;
//    if (Delay.Elapsed(&ITimer, IBlinkDelay)) {
//        IsInsideBlink = false;
//        Off();
//    }
//}

// ============================= LedSmooth_t ===================================
/*
 * Parameters:
 * GPIO_TypeDef *AGpio, uint16_t APinNumber,
 * TIM_TypeDef* ATimer, uint16_t ATopValue, uint16_t APrescaler,
 * uint8_t AChannelNumber, bool InvertedPolarity
 */
/*void LedSmooth_t::Init(GPIO_TypeDef *AGpio, uint16_t APinNumber, TIM_TypeDef* ATimer, uint16_t ATopValue, uint16_t APrescaler, uint8_t AChannelNumber, bool InvertedPolarity) {
    // ==== Gpio ====
    klGpioSetupByN(AGpio, APinNumber, GPIO_Mode_AF_PP);
    // ==== Timer ====
    IPwmTimer = ATimer;
    // Clock
    if      (IPwmTimer == TIM1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    else if (IPwmTimer == TIM2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    else if (IPwmTimer == TIM3) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    else if (IPwmTimer == TIM4) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    // Timebase
    IPwmTimer->CR1 = 0x01;  // Enable timer
    IPwmTimer->CR2 = 0;
    IPwmTimer->PSC = APrescaler;
    IPwmTimer->ARR = ATopValue; // Autoreload register
    if (IPwmTimer == TIM1) IPwmTimer->BDTR = 0xC000;    // Enable output
    // PWM channel
    switch (AChannelNumber) {
        case 1:
            ICCR = (uint16_t*)(&IPwmTimer->CCR1);
            IPwmTimer->CCMR1 |= 0x60;  // Output, PWM mode 1
            IPwmTimer->CCER  |= InvertedPolarity? 0x0003 : 0x0001;  // Compare output 1 enabled, OC1 is active low/high
            break;
        case 2:
            ICCR = (uint16_t*)(&IPwmTimer->CCR2);
            IPwmTimer->CCMR1 |= 0x6000;
            IPwmTimer->CCER  |= InvertedPolarity? 0x0030 : 0x0010;
            break;
        case 3:
            ICCR = (uint16_t*)(&IPwmTimer->CCR3);
            IPwmTimer->CCMR2 |= 0x60;
            IPwmTimer->CCER  |= InvertedPolarity? 0x0300 : 0x0100;
            break;
        case 4:
            ICCR = (uint16_t*)(&IPwmTimer->CCR4);
            IPwmTimer->CCMR2 |= 0x6000;
            IPwmTimer->CCER  |= InvertedPolarity? 0x3000 : 0x1000;
            break;
        default: break;
    }
}

void LedSmooth_t::RampUp(void) {
    State = lsUp;
    Delay.Reset(&IDelayTmr);
    ISetupDelay();
    //klPrintf("1: %u\r", IDelay);
}
void LedSmooth_t::RampDown(void) {
    State = lsDown;
    Delay.Reset(&IDelayTmr);
    ISetupDelay();
}

void LedSmooth_t::Task() {
    if (State == lsUp) {
        // Check if top achieved
        if (IValue == Top()) { State = lsOn; return; }
        // Otherwise, check if delay elapsed
        if (Delay.Elapsed(&IDelayTmr, IDelay)) {
            *ICCR = IValue++;
            ISetupDelay();
        }
    }
    else if (State == lsDown) {
        // Check if bottom achieved
        if (IValue == 0) { State = lsOff; return; }
        // Otherwise, check if delay elapsed
        if (Delay.Elapsed(&IDelayTmr, IDelay)) {
            *ICCR = --IValue;
            ISetupDelay();
        }
    }
}
*/

// ============================= RGBLed_t ======================================
LedRGB_t Led;
// ==== LED Thread ====
static WORKING_AREA(waLedThread, 128);
__attribute__((noreturn))
static void LedThread(void *arg) {
    chRegSetThreadName("Led");
    while(1) Led.Task();
}

void LedRGB_t::Init() {
    // ==== GPIO ====
	PinSetupAlterFuncOutput(GPIOA, 8,  omPushPull);
	PinSetupAlterFuncOutput(GPIOA, 10, omPushPull);
	PinSetupAlterFuncOutput(GPIOA, 11, omPushPull);
    // ==== Timer ====
	LED_RCC_EN();
    // ==== Timebase and general ====
	LED_TIM->CR1 = 0x01;       // Enable timer, set clk division to 0, AutoReload not buffered
	LED_TIM->CR2 = 0;          // Output Idle State
	LED_TIM->PSC = 0;          // No clock division
	LED_TIM->ARR = 255;        // Autoreload register: top value of PWM
    // ==== Outputs ====
	LED_TIM->BDTR = TIM_BDTR_MOE | TIM_BDTR_AOE;    // Enable output
	const uint16_t OutCmpBits = 0b01100000;   // output, PWM1
	LED_TIM->CCMR1 = OutCmpBits;   // CCR1
	LED_TIM->CCMR2 = (OutCmpBits << 8) | OutCmpBits;    // CCR3 & CCR4
	LED_TIM->CCER = TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    // Initial values
    SetColor(clBlack);
    INeededColor = clBlack;
    // Thread
    PThread = chThdCreateStatic(waLedThread, sizeof(waLedThread), NORMALPRIO, (tfunc_t)LedThread, NULL);
}

void LedRGB_t::SetColorSmoothly(Color_t AColor) {
    INeededColor = AColor;
    if(IsSleeping) {
        chSysLock();
        chSchWakeupS(PThread, 0);
        IsSleeping = false;
        chSysUnlock();
    }
}

void LedRGB_t::Task(void) {
    if(ICurrentColor == INeededColor) {
        chSysLock();
        IsSleeping = true;
        chSchGoSleepS(THD_STATE_SUSPENDED);
        chSysUnlock();
    }
    else {
        chThdSleepMilliseconds(LED_SETUP_DELAY_MS);
        // Red channel
        if (ICurrentColor.Red != INeededColor.Red) {
            if(INeededColor.Red < ICurrentColor.Red) ICurrentColor.Red--;
            else ICurrentColor.Red++;
        }
        // Green channel
        if (ICurrentColor.Green != INeededColor.Green) {
            if(INeededColor.Green < ICurrentColor.Green) ICurrentColor.Green--;
            else ICurrentColor.Green++;
        }
        // Blue channel
        if (ICurrentColor.Blue != INeededColor.Blue) {
            if(INeededColor.Blue < ICurrentColor.Blue) ICurrentColor.Blue--;
            else ICurrentColor.Blue++;
        }
        SetColor(ICurrentColor);
    }
}
