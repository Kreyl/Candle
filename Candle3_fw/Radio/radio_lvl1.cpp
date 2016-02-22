/*
 * radio_lvl1.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#include "radio_lvl1.h"
#include "evt_mask.h"
#include "application.h"
#include "cc2500.h"
#include "cmd_uart_f10x.h"
#include <cstdlib>

//#define TX
#ifndef TX
#define RX
#endif

#define DBG_PINS

#ifdef DBG_PINS
#define DBG_GPIO1   GPIOB
#define DBG_PIN1    10
#define DBG1_SET()  PinSet(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinClear(DBG_GPIO1, DBG_PIN1)
#endif

rLevel1_t Radio;

#if 1 // ================================ Task =================================
static WORKING_AREA(warLvl1Thread, 256);
__attribute__((noreturn))
static void rLvl1Thread(void *arg) {
    chRegSetThreadName("rLvl1");
    while(true) Radio.ITask();
}

void rLevel1_t::ITask() {
#ifdef TX
    PktTx.R = 0;
    PktTx.G = 255;
    PktTx.B = 0;
    DBG1_SET();
    CC.TransmitSync(&PktTx);
    DBG1_CLR();

#elif defined RX
    // ======== RX cycle ========
    int8_t Rssi;
    uint8_t RxRslt = CC.ReceiveSync(RX_DURATION_MS, &PktRx, &Rssi);
    if(RxRslt == OK) {
        Uart.Printf("Rx: %u %u %u; Rssi=%d\r", PktRx.R, PktRx.G, PktRx.B, Rssi);
        if((PktRx.AppID == R_APP_ID) and (Rssi > RSSI_MIN_DBM)) App.SendEvtRx(PktRx.R, PktRx.G, PktRx.B);
    }
    CC.Sleep();
    chThdSleepMilliseconds(SLEEP_DURATION_MS);
#endif
}
#endif // task

#if 1 // ============================ Init =====================================
void rLevel1_t::Init() {
#ifdef DBG_PINS
    PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull);
#endif
    // Init radioIC
    CC.Init();
    CC.SetTxPower(ccPwr0dBm);
    CC.SetChannel(RCHANNEL);
    CC.SetPktSize(RPKT_LEN);
    // Variables
    PktTx.AppID = R_APP_ID;
    // Thread
    chThdCreateStatic(warLvl1Thread, sizeof(warLvl1Thread), HIGHPRIO, (tfunc_t)rLvl1Thread, NULL);
}
#endif
