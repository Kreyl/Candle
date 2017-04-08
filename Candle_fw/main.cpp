/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "SimpleSensors.h"
#include "buttons.h"
#include "board.h"
#include "led.h"
#include "Sequences.h"
#include "radio_lvl1.h"
#include "kl_adc.h"

#if 1 // ======================== Variables and defines ========================
App_t App;

// EEAddresses
#define EE_ADDR_DEVICE_ID       0

static uint8_t ISetID(int32_t NewID);
Eeprom_t EE;
void ReadIDfromEE();

LedRGBwPower_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN, LED_EN_PIN };

// ==== Timers ====
static TmrKL_t TmrEverySecond {MS2ST(1000), EVT_EVERY_SECOND, tktPeriodic};
static TmrKL_t TmrRxTableCheck {MS2ST(2007), EVT_RXCHECK, tktPeriodic};
//static int32_t TimeS;
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V2);
//    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();
    App.InitThread();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN);
    ReadIDfromEE();
    Uart.Printf("\r%S %S; ID=%u\r", APP_NAME, BUILD_TIME, App.ID);
//    Uart.Printf("ID: %X %X %X\r", GetUniqID1(), GetUniqID2(), GetUniqID3());
//    if(Sleep::WasInStandby()) {
//        Uart.Printf("WasStandby\r");
//        Sleep::ClearStandbyFlag();
//    }
    Clk.PrintFreqs();
//    RandomSeed(GetUniqID3());   // Init random algorythm with uniq ID

    Led.Init();
//    Led.SetupSeqEndEvt(chThdGetSelfX(), EVT_LED_SEQ_END);
#if BTN_ENABLED
    PinSensors.Init();
#endif
//    Adc.Init();

    // ==== Time and timers ====
//    TmrEverySecond.InitAndStart();
    TmrRxTableCheck.InitAndStart();

    // ==== Radio ====
    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqStart);
    else Led.StartOrRestart(lsqFailure);
    chThdSleepMilliseconds(1008);

    // Main cycle
    App.ITask();
}

__noreturn
void App_t::ITask() {
    while(true) {
        __unused eventmask_t Evt = chEvtWaitAny(ALL_EVENTS);
//        if(Evt & EVT_EVERY_SECOND) {
//            TimeS++;
//        }

#if BTN_ENABLED
        if(Evt & EVT_BUTTONS) {
            Uart.Printf("Btn\r");
            BtnEvtInfo_t EInfo;
            while(BtnGetEvt(&EInfo) == OK) {
                if(EInfo.Type == bePress) {
                    Sheltering.TimeOfBtnPress = TimeS;
                    Sheltering.ProcessCChangeOrBtn();
                } // if Press
            } // while
        }
#endif

//        if(Evt & EVT_RX) {
////            int32_t TimeRx = Radio.PktRx.Time;
//            Uart.Printf("RX\r");
//        }

        if(Evt & EVT_RXCHECK) {
            if(Radio.RxTable.GetCount() != 0) {
                Led.StartOrContinue(lsqTheyAreNear);
                Radio.RxTable.Clear();
            }
            else Led.StartOrContinue(lsqTheyDissapeared);
        }


#if 0 // ==== Led sequence end ====
        if(Evt & EVT_LED_SEQ_END) {
        }
#endif
        //        if(Evt & EVT_OFF) {
        ////            Uart.Printf("Off\r");
        //            chSysLock();
        //            Sleep::EnableWakeup1Pin();
        //            Sleep::EnterStandby();
        //            chSysUnlock();
        //        }

#if UART_RX_ENABLED
        if(Evt & EVT_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
            Uart.SignalCmdProcessed();
        }
#endif

    } // while true
} // App_t::ITask()

#if UART_RX_ENABLED // ================= Command processing ====================
void App_t::OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }

    else if(PCmd->NameIs("Version")) PShell->Printf("%S %S\r", APP_NAME, BUILD_TIME);

    else if(PCmd->NameIs("GetID")) PShell->Reply("ID", App.ID);

    else if(PCmd->NameIs("SetID")) {
        uint8_t b, r;
        if((r = PCmd->GetNext<uint8_t>(&b)) == retvOk) {
            r = ISetID(b);
        }
        PShell->Ack(r);
    }

    else PShell->Ack(retvCmdUnknown);
}
#endif

#if 1 // =========================== ID management =============================
void ReadIDfromEE() {
    App.ID = EE.Read32(EE_ADDR_DEVICE_ID);  // Read device ID
    if(App.ID < ID_MIN or App.ID > ID_MAX) {
        Uart.Printf("\rUsing default ID\r");
        App.ID = ID_DEFAULT;
    }
}

uint8_t ISetID(int32_t NewID) {
    if(NewID < ID_MIN or NewID > ID_MAX) return retvFail;
    uint8_t rslt = EE.Write32(EE_ADDR_DEVICE_ID, NewID);
    if(rslt == retvOk) {
        App.ID = NewID;
        Uart.Printf("New ID: %u\r", App.ID);
        return retvOk;
    }
    else {
        Uart.Printf("EE error: %u\r", rslt);
        return retvFail;
    }
}
#endif
