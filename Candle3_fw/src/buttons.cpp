/*
 * keys.cpp
 *
 *  Created on: 07.02.2013
 *      Author: kreyl
 */

#include "buttons.h"
#include "ch.h"

#define TICK_CNT    (BTN_TICK_PERIOD_MS / BTN_POLL_PERIOD_MS)

Buttons_t Btn;

// ============================= Single button =================================
void Btn_t::Init(GPIO_TypeDef *Gpio, uint16_t Pin) {
    PGpio = Gpio;
    IPin = Pin;
    PinSetupIn(Gpio, Pin, pudPullUp);
}

void Btn_t::CheckTransition() {
    if(!PinIsSet(PGpio, IPin) and !Pressed) { // Pressed
        Pressed = true;
        Evt = evtPressed;
    }
    if(PinIsSet(PGpio, IPin) and Pressed) { // Released
        Pressed = false;
        Evt = evtReleased;
    }
}

// ================================ Buttons ====================================
static WORKING_AREA(waBtnThread, 128);
__attribute__((noreturn))
static void BtnThread(void *arg) {
    chRegSetThreadName("Btn");
    while(1) Btn.Task();
}

void Buttons_t::Init() {
//    chEvtInit(&EvtSrc);
    Up.Init  (GPIOB, 6);
    Down.Init(GPIOB, 7);
    Tx.Init  (GPIOA, 1);
    // Create and start thread
    chThdCreateStatic(waBtnThread, sizeof(waBtnThread), NORMALPRIO, (tfunc_t)BtnThread, NULL);
}

void Buttons_t::Shutdown() {
    Up.Shutdown();
    Down.Shutdown();
    Tx.Shutdown();
}

void Buttons_t::Task() {
       chThdSleepMilliseconds(BTN_POLL_PERIOD_MS);
       // Check transition from pressed to depressed and vice versa
       Up.CheckTransition();
       Down.CheckTransition();
       Tx.CheckTransition();

       // Check if long-press tick occured
       if((Up.Evt == evtPressed) or (Down.Evt == evtPressed)) TickCounter = 0;
       if(TickCounter++ >= TICK_CNT) {
           if((Up.Evt == evtNone) and Up.Pressed) Up.Evt = evtTick;
           if((Down.Evt == evtNone) and Down.Pressed) Down.Evt = evtTick;
           TickCounter = 0;
       }

       // Check if event occcured
       if((Up.Evt != evtNone) or (Down.Evt != evtNone) or (Tx.Evt != evtNone)) {
//           Uart.Printf("%u %u %u\r", Up.Evt, Down.Evt, Tx.Evt);
           if(EvtCallback != NULL) EvtCallback();
//           chEvtBroadcast(&EvtSrc);
       }
   }
