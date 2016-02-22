/*
 * led.h
 *
 *  Created on: 03.11.2011
 *      Author: g.kruglov
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f10x.h"
#include "ch.h"
#include "hal.h"
#include "kl_lib_f100.h"
/*
class Led_t {
protected:
    uint16_t IPinNumber;
    GPIO_TypeDef* IGPIO;
public:
    void Init(GPIO_TypeDef *PGpioPort, uint16_t APinNumber) { IGPIO = PGpioPort; IPinNumber = APinNumber; PinSetupOut(PGpioPort, APinNumber, GPIO_Mode_Out_PP); }
    void On(void)  { PinSet  (IGPIO, IPinNumber); }
    void Off(void) { PinClear(IGPIO, IPinNumber); }
};

class LedBlink_t : public Led_t {
private:
    uint32_t ITimer, IBlinkDelay;
    bool IsInsideBlink;
public:
    void Disable(void) { Off(); IsInsideBlink = false; }
    void Blink(uint32_t ABlinkDelay);
    void Task(void);
};

class LedBlinkInverted_t {
private:
    uint32_t ITimer, IBlinkDelay;
    bool IsInsideBlink;
    uint16_t IPinNumber;
    GPIO_TypeDef* IGPIO;
public:
    void Init(GPIO_TypeDef *PGpioPort, uint16_t APinNumber) { IGPIO = PGpioPort; IPinNumber = APinNumber; klGpioSetupByN(PGpioPort, APinNumber, GPIO_Mode_Out_OD); }
    void On(void)  { klGpioClearByN(IGPIO, IPinNumber); }
    void Off(void) { klGpioSetByN  (IGPIO, IPinNumber); }
    void Disable(void) { Off(); IsInsideBlink = false; }
    void Blink(uint32_t ABlinkDelay) {
        IsInsideBlink = true;
        IBlinkDelay = ABlinkDelay;
        On();
        Delay.Reset(&ITimer);
    }
    void Task(void) {
        if (!IsInsideBlink) return;
        if (Delay.Elapsed(&ITimer, IBlinkDelay)) {
            IsInsideBlink = false;
            Off();
        }
    }
};
*/

/*
 * Smooth on & off.
 * Class is stand-alone as ordinal On & Off methods will not work with Alternate Function pin.
 */
/*
enum LedState_t {lsOff, lsOn, lsUp, lsDown};
class LedSmooth_t {
private:
    uint32_t IDelayTmr, IDelay;
    TIM_TypeDef* IPwmTimer;
    uint16_t *ICCR, IValue;
    void ISetupDelay(void) {
        if (IValue < 11) IDelay = 81;
        else if (IValue < 27) IDelay = 36;
        else IDelay = 9;
    }
public:
    LedState_t State;
    void Init(GPIO_TypeDef *AGpio, uint16_t APinNumber, TIM_TypeDef* ATimer, uint16_t ATopValue, uint16_t APrescaler, uint8_t AChannelNumber, bool InvertedPolarity);
    void Task(void);
    void Set(uint16_t AValue) { *ICCR = AValue; IValue = AValue; }
    uint16_t Top(void)        { return (IPwmTimer->ARR); }
    void On(void)       { Set(Top()); State = lsOn; }
    void Off(void)      { Set(0); State = lsOff; }
    void RampUp(void);
    void RampDown(void);
};
*/

// Colors
struct Color_t {
    uint8_t Red, Green, Blue;
    bool operator == (const Color_t AColor) { return ((Red == AColor.Red) and (Green == AColor.Green) and (Blue == AColor.Blue)); }
    //bool operator != (const Color_t AColor) { return ((this->Red != AColor.Red) || (this->Green != AColor.Green) || (this->Blue != AColor.Blue)); }
} __attribute__ ((__packed__));
#define clBlack     ((Color_t){0,   0,   0})
#define clRed       ((Color_t){255, 0,   0})
#define clGreen     ((Color_t){0,   255, 0})
#define clBlue      ((Color_t){0,   0,   255})
#define clYellow    ((Color_t){255, 255, 0})
#define clViolet    ((Color_t){255, 0, 255})
#define clCyan      ((Color_t){0, 255, 255})
#define clWhite     ((Color_t){255, 255, 255})

/*
 * RGB LED
 * Provides smooth color change.
 */
#define LED_SETUP_DELAY_MS  7

#define LED_TIM         TIM1
#define LED_RED_CCR     CCR3
#define LED_GREEN_CCR   CCR1
#define LED_BLUE_CCR    CCR4
#define LED_RCC_EN()    rccEnableTIM1()

class LedRGB_t {
private:
	Color_t ICurrentColor, INeededColor;
	Thread *PThread;
	bool IsSleeping;
	void ISetRed  (uint8_t AValue) {LED_TIM->LED_RED_CCR   = AValue;}
	void ISetGreen(uint8_t AValue) {LED_TIM->LED_GREEN_CCR = AValue;}
	void ISetBlue (uint8_t AValue) {LED_TIM->LED_BLUE_CCR  = AValue;}
public:
	void Init();
	void Task();
	void SetColor(Color_t AColor) {
		ISetRed(AColor.Red);
		ISetGreen(AColor.Green);
		ISetBlue(AColor.Blue);
		ICurrentColor = AColor;
	}
	void SetColorSmoothly(Color_t AColor);
	bool IsOff() { return (ICurrentColor == INeededColor) and (ICurrentColor == clBlack); }
//	void Blink(uint32_t ABlinkDelay, Color_t AColor) {
//	        IsInsideBlink = true;
//	        IBlinkDelay = ABlinkDelay;
//	        INeededColor = AColor;
//	        SetColor(AColor);
//	        Delay.Reset(&ITimer);
//	    }
};

extern LedRGB_t Led;


#endif /* LED_H_ */
