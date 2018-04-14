/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "SimpleSensors.h"
#include "buttons.h"
#include "board.h"
#include "led.h"
#include "Sequences.h"
#include "radio_lvl1.h"
//#include "kl_adc.h"
#include "MsgQ.h"
#include "main.h"
#include "ColorTable.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
static void ITask();
static void OnCmd(Shell_t *PShell);

// EEAddresses
#define EE_ADDR_DEVICE_ID       0
int32_t ID;

static uint8_t ISetID(int32_t NewID);
void ReadIDfromEE();

LedRGBwPower_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN, LED_EN_PIN };
LedRGBChunk_t lsqSetColor[] = {
        {csSetup, 90, clRed},
        {csEnd},
};

// ==== Timers ====
static TmrKL_t TmrEverySecond {MS2ST(1000), evtIdEverySecond, tktPeriodic};
//static TmrKL_t TmrRxTableCheck {MS2ST(2007), evtIdCheckRxTable, tktPeriodic};

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
    EvtQMain.Init();

    // ==== Init hardware ====
    Uart.Init(115200);
    ReadIDfromEE();
    Printf("\r%S %S; ID=%u\r", APP_NAME, BUILD_TIME, ID);
//    Uart.Printf("ID: %X %X %X\r", GetUniqID1(), GetUniqID2(), GetUniqID3());
//    if(Sleep::WasInStandby()) {
//        Uart.Printf("WasStandby\r");
//        Sleep::ClearStandbyFlag();
//    }
    Clk.PrintFreqs();
//    RandomSeed(GetUniqID3());   // Init random algorythm with uniq ID

    Led.Init();
    lsqSetColor[0].Color = *ColorTable.GetCurrent();
#if BUTTONS_ENABLED
    SimpleSensors::Init();
#endif
//    Adc.Init();

    // ==== Time and timers ====
//    TmrEverySecond.InitAndStart();
//    TmrRxTableCheck.InitAndStart();

    // ==== Radio ====
    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqStart);
    else Led.StartOrRestart(lsqFailure);
    chThdSleepMilliseconds(2700);
    // Show current color
    Led.StartOrRestart(lsqSetColor);

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdEverySecond:
//                TimeS++;
                break;

#if BUTTONS_ENABLED
            case evtIdButtons:
//                Printf("Btn %u\r", Msg.BtnEvtInfo.BtnID[0]);
                switch(Msg.BtnEvtInfo.Type) {
                    case beShortPress:
                        if(Msg.BtnEvtInfo.BtnID[0] == BTN_TX_INDX) {
                            Printf("Tx On\r");
                        }
                        // No break is intentional
                    case beRepeat:
                        if(Msg.BtnEvtInfo.BtnID[0] == BTN_UP_INDX) {
                            Printf("Up\r");
                        }
                        else if(Msg.BtnEvtInfo.BtnID[0] == BTN_DOWN_INDX) {
                            Printf("Down\r");
                        }
                        break;
                    case beRelease:
                        if(Msg.BtnEvtInfo.BtnID[0] == BTN_TX_INDX) {
                            Printf("Tx Off\r");
                        }
                        break;
                    case beCombo:
                        if((Msg.BtnEvtInfo.BtnID[0] == BTN_UP_INDX and Msg.BtnEvtInfo.BtnID[1] == BTN_DOWN_INDX) or
                           (Msg.BtnEvtInfo.BtnID[0] == BTN_DOWN_INDX and Msg.BtnEvtInfo.BtnID[1] == BTN_UP_INDX)) {
                            Printf("Combo\r");
                        }
                        break;
                    default: break;
                } // switch
                break;
#endif

#if UART_RX_ENABLED
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;
#endif
            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // Switch
    } // while true
} // ITask()


#if UART_RX_ENABLED // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
    Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }
    else if(PCmd->NameIs("Version")) PShell->Printf("%S %S\r", APP_NAME, BUILD_TIME);

    else if(PCmd->NameIs("GetID")) PShell->Reply("ID", ID);

    else if(PCmd->NameIs("SetID")) {
        if(PCmd->GetNext<int32_t>(&ID) != retvOk) { PShell->Ack(retvCmdError); return; }
        uint8_t r = ISetID(ID);
//        RMsg_t msg;
//        msg.Cmd = R_MSG_SET_CHNL;
//        msg.Value = ID2RCHNL(ID);
//        Radio.RMsgQ.SendNowOrExit(msg);
        PShell->Ack(r);
    }

    else PShell->Ack(retvCmdUnknown);
}
#endif

#if 1 // =========================== ID management =============================
void ReadIDfromEE() {
    ID = EE::Read32(EE_ADDR_DEVICE_ID);  // Read device ID
    if(ID < ID_MIN or ID > ID_MAX) {
        Printf("\rUsing default ID\r");
        ID = ID_DEFAULT;
    }
}

uint8_t ISetID(int32_t NewID) {
    if(NewID < ID_MIN or NewID > ID_MAX) return retvFail;
    uint8_t rslt = EE::Write32(EE_ADDR_DEVICE_ID, NewID);
    if(rslt == retvOk) {
        ID = NewID;
        Printf("New ID: %u\r", ID);
        return retvOk;
    }
    else {
        Printf("EE error: %u\r", rslt);
        return retvFail;
    }
}
#endif
