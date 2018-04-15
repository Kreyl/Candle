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

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
static void ITask();
static void OnCmd(Shell_t *PShell);

// EEAddresses
#define EE_ADDR_DEVICE_ID   0
#define EE_ADDR_CLR         4
int32_t ID;
static uint8_t ISetID(int32_t NewID);
void ReadIDfromEE();

LedRGBwPower_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN, LED_EN_PIN };
ColorHSV_t ClrHSV{0, 100, 100};
Color_t ClrRGB;

LedRGBChunk_t lsqFadeIn[] = {
        {csSetup, 360, clBlack},
        {csEnd},
};

LedRGBChunk_t lsqFadeOut[] = {
        {csSetup, 360, clBlack},
        {csEnd},
};

LedRGBChunk_t lsqFadeInBlue[] = {
        {csSetup, 360, clDarkBlue},
        {csEnd},
};


enum State_t {stateOff, stateFadeIn, stateActive, stateFadeOut, statePreOff};
static State_t State = stateOff;

static void BtnHandlerActive(BtnEvtInfo_t BtnEvtInfo);
static void BtnHandlerPreOff(BtnEvtInfo_t BtnEvtInfo);
static void LoadColor();
static void SaveColor();
static void EnterOff();

// ==== Timers ====
//static TmrKL_t TmrEverySecond {MS2ST(1000), evtIdEverySecond, tktPeriodic};
//static TmrKL_t TmrRxTableCheck {MS2ST(2007), evtIdCheckRxTable, tktPeriodic};
static TmrKL_t TmrOff {MS2ST(9000), evtIdTmrOff, tktOneShot};

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
    Led.SetupSeqEndEvt(EvtMsg_t(evtIdLedSeqEnd));

#if BUTTONS_ENABLED
    SimpleSensors::Init();
#endif
//    Adc.Init();

    // ==== Time and timers ====
//    TmrEverySecond.InitAndStart();
//    TmrRxTableCheck.InitAndStart();

    // ==== Radio ====
    if(Radio.Init() != retvOk) {
        Led.StartOrRestart(lsqFailure);
        chThdSleepMilliseconds(3600);
    }

    // Show current color
    LoadColor();
    ClrRGB = ClrHSV.ToRGB();
    lsqFadeIn[0].Color = ClrRGB;
    Led.StartOrRestart(lsqFadeIn);
    State = stateFadeIn;

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
                if(State == stateActive) BtnHandlerActive(Msg.BtnEvtInfo);
                else if(State == statePreOff) BtnHandlerPreOff(Msg.BtnEvtInfo);
                break;
#endif

            case evtIdLedSeqEnd:
                if(State == stateFadeIn) State = stateActive;
                else if(State == stateFadeOut) {
                    TmrOff.StartOrRestart();
                    State = statePreOff;
                }
                break;

            case evtIdTmrOff:
                SaveColor();
                EnterOff();
                break;

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

static void BtnHandlerActive(BtnEvtInfo_t BtnEvtInfo) {
    switch(BtnEvtInfo.Type) {
        case beShortPress:
            if(BtnEvtInfo.BtnID[0] == BTN_TX_INDX) {
                Printf("Tx On\r");
                Radio.MustTx = true;
            }
            // No break is intentional
        case beRepeat:
            if(BtnEvtInfo.BtnID[0] == BTN_UP_INDX) {
                Printf("Up\r");
                ClrHSV.H++;
                if(ClrHSV.H > CLR_HSV_H_MAX) ClrHSV.H = 0;
                ClrRGB = ClrHSV.ToRGB();
                lsqFadeIn[0].Color = ClrRGB;
                Led.StartOrRestart(lsqFadeIn);
            }
            else if(BtnEvtInfo.BtnID[0] == BTN_DOWN_INDX) {
                Printf("Down\r");
                if(ClrHSV.H == 0) ClrHSV.H = CLR_HSV_H_MAX;
                else ClrHSV.H--;
                ClrRGB = ClrHSV.ToRGB();
                lsqFadeIn[0].Color = ClrRGB;
                Led.StartOrRestart(lsqFadeIn);
            }
            break;
        case beRelease:
            if(BtnEvtInfo.BtnID[0] == BTN_TX_INDX) {
                Printf("Tx Off\r");
                Radio.MustTx = false;
            }
            break;
        case beCombo:
            if((BtnEvtInfo.BtnID[0] == BTN_UP_INDX and BtnEvtInfo.BtnID[1] == BTN_DOWN_INDX) or
               (BtnEvtInfo.BtnID[0] == BTN_DOWN_INDX and BtnEvtInfo.BtnID[1] == BTN_UP_INDX)) {
                Printf("Combo\r");
                State = stateFadeOut;
                Led.StartOrRestart(lsqFadeOut);
            }
            break;
        default: break;
    } // switch
}

static void BtnHandlerPreOff(BtnEvtInfo_t BtnEvtInfo) {
    switch(BtnEvtInfo.Type) {
        case beShortPress:
            if(BtnEvtInfo.BtnID[0] == BTN_TX_INDX) {
                Printf("FadeTx On\r");
                TmrOff.StartOrRestart();
                Led.StartOrRestart(lsqFadeInBlue);
                Radio.MustTx = true;
            }
            else if(BtnEvtInfo.BtnID[0] == BTN_UP_INDX or BtnEvtInfo.BtnID[0] == BTN_DOWN_INDX) {
                Printf("Fade UpDown\r");
                TmrOff.Stop();
                lsqFadeIn[0].Color = ClrRGB;
                Led.StartOrRestart(lsqFadeIn);
                State = stateActive;
            }
            break;
        case beRelease:
            if(BtnEvtInfo.BtnID[0] == BTN_TX_INDX) {
                Printf("FadeTx Off\r");
                TmrOff.StartOrRestart();
                Led.StartOrRestart(lsqFadeOut);
                Radio.MustTx = false;
            }
            break;
        default: break;
    } // switch
}

void LoadColor() {
    Printf("LoadColor\r");
    ClrHSV.DWord32 = EE::Read32(EE_ADDR_CLR);
    if(ClrHSV.S != CLR_HSV_S_MAX or ClrHSV.V != CLR_HSV_V_MAX) {
        ClrHSV.H = 0;
        ClrHSV.S = CLR_HSV_S_MAX;
        ClrHSV.V = CLR_HSV_S_MAX;
    }
}
void SaveColor() {
    uint32_t tmp = EE::Read32(EE_ADDR_CLR);
    if(tmp == ClrHSV.DWord32) Printf("Color not changed\r");
    else {
        uint8_t rslt = EE::Write32(EE_ADDR_CLR, ClrHSV.DWord32);
        if(rslt == retvOk) Printf("Color saved\r");
        else Printf("EE error: %u\r", rslt);
    }
}

void EnterOff() {
    Printf("EnterOff\r");
    Radio.MustTx = false;
    chThdSleepMilliseconds(45);
    while(GetBtnState(0) != BTN_IDLE_STATE) {
        chThdSleepMilliseconds(18);
    }
    chSysLock();
    Sleep::EnableWakeup1Pin();
    Sleep::EnterStandby();
    chSysUnlock();
    Printf("AfterSleep\r");
}


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
