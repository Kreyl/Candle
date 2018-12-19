#include "SimpleSensors.h"
#include "buttons.h"
#include "board.h"
#include "led.h"
#include "Sequences.h"
#include "radio_lvl1.h"
//#include "kl_adc.h"
#include "MsgQ.h"
#include "main.h"
#include "acg_lsm6ds3.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
static void ITask();
static void OnCmd(Shell_t *PShell);

// EEAddresses
#define EE_ADDR_DEVICE_ID   0
#define EE_ADDR_CLR         4
int32_t ID;
static uint8_t ISetID(int32_t NewID);
void ReadIDfromEE();

//LedRGBwPower_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN, LED_EN_PIN };
//ColorHSV_t ClrHSV{0, 100, 100};
//Color_t ClrRGB;

LedSmooth_t Led{LED_B_PIN};

// ==== Timers ====
//static TmrKL_t TmrEverySecond {MS2ST(1000), evtIdEverySecond, tktPeriodic};
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();
    EvtQMain.Init();

    // ==== Init hardware ====
    Uart.Init();
    ReadIDfromEE();
    Printf("\r%S %S; ID=%u\r", APP_NAME, XSTRINGIFY(BUILD_TIME), ID);
    Clk.PrintFreqs();

    Led.Init();
    SimpleSensors::Init();
    Acg.Init();

    // ==== Time and timers ====
//    TmrEverySecond.InitAndStart();

    // ==== Radio ====
    if(Radio.Init() == retvOk) Led.StartOrRestart(lsqIdle);
    else Led.StartOrRestart(lsqFailure);
    chThdSleepMilliseconds(3600);

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdButtons:
                Printf("Btn %u\r", Msg.BtnEvtInfo.BtnID[0]);
                break;

            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // Switch
    } // while true
} // ITask()

#if 1 // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
    Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }
    else if(PCmd->NameIs("Version")) PShell->Print("%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));

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
