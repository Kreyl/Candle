/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "kl_lib_f100.h"
#include "ch.h"
#include "hal.h"
#include "led.h"
#include "buttons.h"
#include "color_table.h"
#include "cc2500.h"

#define CC_CHANNEL  11

static inline void Init();
static inline void ProcessBtnEvt();
static uint8_t ColorIndx;
static rPkt_t Pkt;
//static EventListener EvtLstnrRadioRx, EvtListenerKeys;
static enum State_t {sIdle, sColorEdit, sFade, sCharging} State;

#define WAIT_DURATION_MS    999
#define PKT_RX_DURATION_MS  4
#define RX_RETRY_CNT        4

#define FADE_CMD            0xFE    // Do not use FF, as CC's CRC does not let it through

//#define EVTMASK_KEYS        EVENT_MASK(2)

/* TODO:
 * 1) Radio Rx\Tx energy saving mode, work time after time, not continuously
 * 2) Measure battery, standby when discharged
 * 3) Charge indication and so.
 * 4) Energy saving
 */

int main(void) {
    // ==== Init clock system ====
    Clk.SetupBusDividers(ahbDiv8, apbDiv1, apbDiv1);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    Init();

    // Events
//    uint32_t EvtMsk;
//    Btn.RegisterEvt(&EvtListenerKeys, EVTMASK_KEYS);
    // Application cycle
    while(TRUE) {
        chThdSleepMilliseconds(999);
    } // while
}

// ==== Radio Thread ====
static WORKING_AREA(waRadioThread, 128);
__attribute__((noreturn))
static void RadioThread(void *arg) {
    chRegSetThreadName("Radio");
    while(1) {
        // Transmit continuously if Tx pressed
        if(Btn.Tx.Pressed) {
            Pkt.Indx = (State == sFade)? FADE_CMD : ColorIndx;
            //Uart.Printf("Tx %u\r", Pkt.Indx);
            CC.TransmitSync(&Pkt);
        }
        else {
            // If Up or Down pressed, do nothing
            if(Btn.Up.Pressed or Btn.Down.Pressed) chThdSleepMilliseconds(45);
            // Receive if none of the buttons pressed
            else {
                // Try to receive several times
                for(uint8_t i=0; i<RX_RETRY_CNT; i++) {
                    Uart.Printf("Rx\r");
                    uint8_t Result = CC.ReceiveSync(PKT_RX_DURATION_MS, &Pkt);
                    if(Result == OK) {
                        Uart.Printf("RSSI=%d; %u\r", Pkt.RSSI, Pkt.Indx);
                        if(Pkt.Indx == FADE_CMD) {  // Enter fade
                            State = sFade;
                            Led.SetColorSmoothly(clBlack);
                            break;
                        }
                        else if(Pkt.Indx < COLOR_COUNT) {
                            State = sIdle;
                            ColorIndx = Pkt.Indx;
                            Led.SetColorSmoothly(ColorTable[ColorIndx]);
                            break;
                        } // if fade
                    } // if rx ok
                } // for
                chThdSleepMilliseconds(WAIT_DURATION_MS);
            } // if not pressed
        } // if Tx is pressed
    } // while 1
}

void Init() {
    JtagDisable();
    State = sIdle;
    Uart.Init(57600);
    ColorIndx = COLOR_COUNT-1;
    Led.Init();
    Led.SetColorSmoothly(ColorTable[ColorIndx]);
    Btn.Init();
    Btn.EvtCallback = ProcessBtnEvt;
    // Radio
    CC.Init();
    CC.SetChannel(CC_CHANNEL);
    CC.SetTxPower(PwrMinus8dBm);
    chThdCreateStatic(waRadioThread, sizeof(waRadioThread), NORMALPRIO, (tfunc_t)RadioThread, NULL);

    Uart.Printf("\rCandle AHB=%u; APB1=%u; APB2=%u\r", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
}

void ProcessBtnEvt() {
    switch(State) {
        case sIdle:
            if((Btn.Up.Evt == evtPressed) and (Btn.Down.Evt == evtPressed)) {     // Both pressed, switch to sFade
                State = sFade;
                Led.SetColorSmoothly(Btn.Tx.Pressed? COLOR_FADE : clBlack);
            }
            else if(Btn.Up.Pressed or Btn.Down.Pressed) State = sColorEdit;   // Any pressed
            break;

        case sColorEdit:
            if(Btn.Up.Pressed and Btn.Down.Pressed) {     // Both pressed, switch to sFade
                State = sFade;
                Led.SetColorSmoothly(Btn.Tx.Pressed? COLOR_FADE : clBlack);
            }
            else if((Btn.Up.Evt == evtReleased) or (Btn.Down.Evt == evtReleased)) State = sIdle;  // Any released
            // Process long hold of button
            else if(Btn.Up.Evt == evtTick) {
                if(ColorIndx < (COLOR_COUNT-1)) ColorIndx++;
                else ColorIndx = 0;
                Led.SetColorSmoothly(ColorTable[ColorIndx]);
            }
            else if(Btn.Down.Evt == evtTick) {
                if(ColorIndx != 0) ColorIndx--;
                else ColorIndx = COLOR_COUNT-1;
                Led.SetColorSmoothly(ColorTable[ColorIndx]);
            }
            break;

        case sFade:
            if((Btn.Up.Evt == evtPressed) or (Btn.Down.Evt == evtPressed)) {
                State = sColorEdit;
                Led.SetColorSmoothly(ColorTable[ColorIndx]);
            }
            else if(Btn.Tx.Evt == evtPressed)  Led.SetColorSmoothly(COLOR_FADE);
            else if(Btn.Tx.Evt == evtReleased) Led.SetColorSmoothly(clBlack);
            break;

        case sCharging: break;  // Do nothing
    } // switch state
    Btn.ClearEvents();
}
