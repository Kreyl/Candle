/*
 * keys.h
 *
 *  Created on: 07.02.2013
 *      Author: kreyl
 */

#ifndef KEYS_H_
#define KEYS_H_

#include "hal.h"
#include "kl_lib_f100.h"

#define BTN_POLL_PERIOD_MS      72
#define BTN_TICK_PERIOD_MS      900

// Button Event
enum BtnEvent_t {evtNone, evtPressed, evtReleased, evtTick};

class Btn_t {
private:
    GPIO_TypeDef *PGpio;
    uint16_t IPin;
public:
    bool Pressed;
    BtnEvent_t Evt;
    void Init(GPIO_TypeDef *Gpio, uint16_t Pin);
    void Shutdown() { PinSetupAnalog(PGpio, IPin); }
    void CheckTransition();
};

class Buttons_t {
private:
//    EventSource EvtSrc;
    uint32_t TickCounter;
public:
    Btn_t Up, Down, Tx;
    void Init();
//    void RegisterEvt(EventListener *PEvtLstnr, uint8_t EvtMask) { chEvtRegisterMask(&EvtSrc, PEvtLstnr, EvtMask); }
    ftVoidVoid EvtCallback;
    void Shutdown();
    inline bool EvtBoth(BtnEvent_t AEvt, Btn_t *PBtn1, Btn_t *PBtn2) { return ((PBtn1->Evt == AEvt) and (PBtn2->Evt == AEvt)); }
    inline bool EvtAny (BtnEvent_t AEvt, Btn_t *PBtn1, Btn_t *PBtn2) { return ((PBtn1->Evt == AEvt) or  (PBtn2->Evt == AEvt)); }
    inline void ClearEvents() { Up.Evt = evtNone; Down.Evt = evtNone; Tx.Evt = evtNone; }
    // Inner use
    inline void Task();
};

extern Buttons_t Btn;


#endif /* KEYS_H_ */
