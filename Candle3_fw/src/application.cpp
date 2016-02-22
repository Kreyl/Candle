/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
//#include "cmd_uart.h"
#include "evt_mask.h"
#include "radio_lvl1.h"
#include "ch.h"
#include "ws2812b.h"

App_t App;

#if 1 // ============================ Timers ===================================
//static VirtualTimer ITmrPillCheck;
void TmrPillCheckCallback(void *p) {
    chSysLockFromIsr();
//    chEvtSignalI(App.PThd, EVTMSK_PILL_CHECK);
//    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlockFromIsr();
}
#endif

#if 1 // ========================= Application =================================
__attribute__((noreturn))
void App_t::ITask() {
    chRegSetThreadName("App");
    while(true) {
//        LedWs.SetCommonColorSmoothly(clGreen, csmOneByOne);
//        chThdSleepMilliseconds(18000);
//        LedWs.SetCommonColorSmoothly(clBlue, csmSimultaneously);
//        chThdSleepMilliseconds(7002);
//        LedWs.SetCommonColorSmoothly(clBlack, csmSimultaneously);
//        chThdSleepMilliseconds(7002);

        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        if(EvtMsk & EVTMSK_RX) LedWs.SetCommonColorSmoothly(IClr, csmOneByOne);
    } // while 1
}

void App_t::Init() {
    // Timers init
    chSysLock();
//    chVTSetI(&ITmrPillCheck, MS2ST(TM_PILL_CHECK_MS),    TmrPillCheckCallback, nullptr);
    chSysUnlock();
}

#endif

#if 0 // ======================= Command processing ============================
#define UART_RPL_BUF_SZ     36
static uint8_t SBuf[UART_RPL_BUF_SZ];

void Ack(uint8_t Result) { Uart.Cmd(0x90, &Result, 1); }

void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    uint16_t w;
    switch(CmdCode) {
        case CMD_PING: Ack(OK); break;

#if 1 // ==== ID and type ====
        case CMD_SET_ID:
            if(Length == 2) {
                w = (PData[0] << 8) | PData[1];
                App.ID = w;
                b = App.EE.Write32(EE_DEVICE_ID_ADDR, App.ID);
                Ack(b);
            }
            else Ack(CMD_ERROR);
            break;
        case CMD_GET_ID:
            SBuf[0] = (App.ID >> 8) & 0xFF;
            SBuf[1] = App.ID & 0xFF;
            Uart.Cmd(RPL_GET_ID, SBuf, 2);
            break;

        case CMD_SET_TYPE:
            if(Length == 1) Ack(App.SetType(PData[0]));
            else Ack(CMD_ERROR);
            break;
        case CMD_GET_TYPE:
            SBuf[0] = (uint8_t)App.Type;
            Uart.Cmd(RPL_GET_TYPE, SBuf, 1);
            break;
#endif

        // ==== Pills ====
        case CMD_PILL_STATE:
            b = PData[0];   // Pill address
            if(b <= 7) SBuf[1] = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_STATE, SBuf, 2);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];
            if(b <= 7) SBuf[1] = PillMgr.Write(PILL_I2C_ADDR, &PData[1], Length-1);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_WRITE, SBuf, 2);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > (UART_RPL_BUF_SZ-2)) b2 = (UART_RPL_BUF_SZ-2);  // Check data size
            if(b <= 7) SBuf[1] = PillMgr.Read(PILL_I2C_ADDR, &SBuf[2], b2);
            SBuf[0] = b;
            if(SBuf[1] == OK) Uart.Cmd(RPL_PILL_READ, SBuf, b2+2);
            else Uart.Cmd(RPL_PILL_READ, SBuf, 2);
            break;

        default: break;
    } // switch
}
#endif
