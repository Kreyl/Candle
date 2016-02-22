/*
 * File:   cc2500.c
 * Author: Laurelindo
 *
 * Edited on 07/21/2013
 */
#include <inttypes.h>
#include "cc2500.h"
#include "cc2500_rf_settings.h"

cc2500_t CC;

void cc2500_t::Init(void) {
    // ==== GPIO ====
    PinSetupOut            (GPIOA, CC_CS,   omPushPull);
    PinSetupAlterFuncOutput(GPIOA, CC_SCK,  omPushPull);
    PinSetupAlterFuncOutput(GPIOA, CC_MISO, omPushPull);
    PinSetupAlterFuncOutput(GPIOA, CC_MOSI, omPushPull);
    PinSetupIn             (GPIOA, CC_GDO0, pudNone);
#ifdef CC_GDO2
    PinSetupIn             (GPIOA, CC_GDO2, pudNone);
#endif
    CsHi();
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=f/2
    ISpi.Setup(CC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv2);
    ISpi.Enable();
    // ==== Init CC ====
    CReset();
    FlushRxFIFO();
    RfConfig();
    PWaitingThread = nullptr;
    State = ccIdle;
    // ==== IRQ ====
    IGdo0.Setup(CC_GPIO, CC_GDO0, ttFalling);
    IGdo0.EnableIrq(IRQ_PRIO_LOW);
//    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // Enable AFIO clock
//    uint8_t tmp = CC_GDO0 / 4;          // Indx of EXTICR register
//    uint8_t Shift = (CC_GDO0 & 0x03) * 4;
//    AFIO->EXTICR[tmp] &= ~((uint32_t)0b1111 << Shift);    // Clear bits and leave clear to select GPIOA
//    tmp = 1<<CC_GDO0;   // IRQ mask
//    // Configure EXTI line
//    EXTI->IMR  |=  tmp;      // Interrupt mode enabled
//    EXTI->EMR  &= ~tmp;      // Event mode disabled
//    EXTI->RTSR &= ~tmp;      // Rising trigger disabled
//    EXTI->FTSR |=  tmp;      // Falling trigger enabled
//    EXTI->PR    =  tmp;      // Clean irq flag
//    nvicEnableVector(EXTI4_IRQn, CORTEX_PRIORITY_MASK(IRQ_PRIO_MEDIUM));
}

void cc2500_t::SetChannel(uint8_t AChannel) {
    while(IState != CC_STB_IDLE) EnterIdle();   // CC must be in IDLE mode
    WriteRegister(CC_CHANNR, AChannel);         // Now set channel
}

void cc2500_t::TransmitSync(void *Ptr) {
    // WaitUntilChannelIsBusy();   // If this is not done, time after time FIFO is destroyed
    while(IState != CC_STB_IDLE) EnterIdle();
    WriteTX((uint8_t*)Ptr, IPktSz);
    // Enter TX and wait IRQ
    chSysLock();
    PWaitingThread = chThdSelf();
    EnterTX();
    chSchGoSleepS(THD_STATE_SUSPENDED);
    chSysUnlock();  // Will be here when IRQ fires
}

void cc2500_t::TransmitAsync(void *Ptr) {
    // WaitUntilChannelIsBusy();   // If this is not done, time after time FIFO is destroyed
    while(IState != CC_STB_IDLE) EnterIdle();
    PWaitingThread = NULL;
    State = ccTransmitting;
    WriteTX((uint8_t*)Ptr, IPktSz);
    EnterTX();
}

// Enter RX mode and wait reception for Timeout_ms.
uint8_t cc2500_t::ReceiveSync(uint32_t Timeout_ms, void *Ptr, int8_t *PRssi) {
    FlushRxFIFO();
    chSysLock();
    PWaitingThread = chThdSelf();
    EnterRX();
    msg_t Rslt = chSchGoSleepTimeoutS(THD_STATE_SUSPENDED, MS2ST(Timeout_ms));
    chSysUnlock();  // Will be here when IRQ will fire, or timeout occur - with appropriate message

    if(Rslt == RDY_TIMEOUT) {   // Nothing received, timeout occured
        EnterIdle();            // Get out of RX mode
        return TIMEOUT;
    }
    else return ReadFIFO(Ptr, PRssi);
}


void cc2500_t::ReceiveAsync() {
    PWaitingThread = NULL;
    State = ccReceiving;
    FlushRxFIFO();
    EnterRX();
}

// Return RSSI in dBm
int8_t cc2500_t::RSSI_dBm(uint8_t ARawRSSI) {
    int16_t RSSI = ARawRSSI;
    if (RSSI >= 128) RSSI -= 256;
    RSSI = (RSSI / 2) - 72;    // see p.35 of cc2500 datasheet
    return RSSI;
}

uint8_t cc2500_t::ReadFIFO(void *Ptr, int8_t *PRssi) {
    uint8_t b, *p = (uint8_t*)Ptr;
     // Check if received successfully
     b = ReadRegister(CC_PKTSTATUS);
     //    Uart.Printf("St: %X  ", b);
     if(b & 0x80) {  // CRC OK
         // Read FIFO
         CsLo();                                            // Start transmission
         BusyWait();                                        // Wait for chip to become ready
         ISpi.ReadWriteByte(CC_FIFO|CC_READ_FLAG|CC_BURST_FLAG); // Address with read & burst flags
         for(uint8_t i=0; i<IPktSz; i++) {                // Read bytes
             b = ISpi.ReadWriteByte(0);
             *p++ = b;
             // Uart.Printf(" %X", b);
         }
         // Receive two additional info bytes
         b = ISpi.ReadWriteByte(0); // RSSI
         ISpi.ReadWriteByte(0);     // LQI
         CsHi();                    // End transmission
         if(PRssi != nullptr) *PRssi = RSSI_dBm(b);
         return OK;
     }
     else return FAILURE;
}

// =========================== Registers & Strobes =============================
uint8_t cc2500_t::ReadRegister (uint8_t ARegAddr) {
    CsLo();                                 // Start transmission
    BusyWait();                             // Wait for chip to become ready
    ISpi.ReadWriteByte(ARegAddr | CC_READ_FLAG); // Transmit header byte
    uint8_t FReply = ISpi.ReadWriteByte(0);      // Read reply
    CsHi();                                 // End transmission
    return FReply;
}
void cc2500_t::WriteRegister (uint8_t ARegAddr, uint8_t AData) {
    CsLo();                     // Start transmission
    BusyWait();                 // Wait for chip to become ready
    ISpi.ReadWriteByte(ARegAddr);    // Transmit header byte
    ISpi.ReadWriteByte(AData);       // Write data
    CsHi();                     // End transmission
}
void cc2500_t::WriteStrobe (uint8_t AStrobe) {
    CsLo();                             // Start transmission
    BusyWait();                         // Wait for chip to become ready
    IState = ISpi.ReadWriteByte(AStrobe);    // Write strobe
    CsHi();                             // End transmission
    IState &= 0b01110000;               // Mask needed bits
}

void cc2500_t::WriteTX(uint8_t* Ptr, uint8_t Length) {
    CsLo();                                                     // Start transmission
    BusyWait();                                                 // Wait for chip to become ready
    ISpi.ReadWriteByte(CC_FIFO|CC_WRITE_FLAG|CC_BURST_FLAG);    // Address with write & burst flags
    uint8_t b;
    //Uart.Printf("TX: ");
    for(uint8_t i=0; i<Length; i++) {
        b = *Ptr++;
        ISpi.ReadWriteByte(b);  // Write bytes
      //  Uart.Printf("%X ", b);
    }
    CsHi();    // End transmission
    //Uart.Printf("\r");
}

// **** Used to setup CC with needed values ****
void cc2500_t::RfConfig(){
    WriteRegister(CC_FSCTRL1,  CC_FSCTRL1_VALUE);    // Frequency synthesizer control.
    WriteRegister(CC_FSCTRL0,  CC_FSCTRL0_VALUE);    // Frequency synthesizer control.
    WriteRegister(CC_FREQ2,    CC_FREQ2_VALUE);      // Frequency control word, high byte.
    WriteRegister(CC_FREQ1,    CC_FREQ1_VALUE);      // Frequency control word, middle byte.
    WriteRegister(CC_FREQ0,    CC_FREQ0_VALUE);      // Frequency control word, low byte.
    WriteRegister(CC_MDMCFG4,  CC_MDMCFG4_VALUE);    // Modem configuration.
    WriteRegister(CC_MDMCFG3,  CC_MDMCFG3_VALUE);    // Modem configuration.
    WriteRegister(CC_MDMCFG2,  CC_MDMCFG2_VALUE);    // Modem configuration.
    WriteRegister(CC_MDMCFG1,  CC_MDMCFG1_VALUE);    // Modem configuration.
    WriteRegister(CC_MDMCFG0,  CC_MDMCFG0_VALUE);    // Modem configuration.
    WriteRegister(CC_DEVIATN,  CC_DEVIATN_VALUE);    // Modem deviation setting (when FSK modulation is enabled).
    WriteRegister(CC_FREND1,   CC_FREND1_VALUE);     // Front end RX configuration.
    WriteRegister(CC_FREND0,   CC_FREND0_VALUE);     // Front end RX configuration.
    WriteRegister(CC_MCSM0,    CC_MCSM0_VALUE);      // Main Radio Control State Machine configuration.
    WriteRegister(CC_FOCCFG,   CC_FOCCFG_VALUE);     // Frequency Offset Compensation Configuration.
    WriteRegister(CC_BSCFG,    CC_BSCFG_VALUE);      // Bit synchronization Configuration.
    WriteRegister(CC_AGCCTRL2, CC_AGCCTRL2_VALUE);   // AGC control.
    WriteRegister(CC_AGCCTRL1, CC_AGCCTRL1_VALUE);   // AGC control.
    WriteRegister(CC_AGCCTRL0, CC_AGCCTRL0_VALUE);   // AGC control.
    WriteRegister(CC_FSCAL3,   CC_FSCAL3_VALUE);     // Frequency synthesizer calibration.
    WriteRegister(CC_FSCAL2,   CC_FSCAL2_VALUE);     // Frequency synthesizer calibration.
    WriteRegister(CC_FSCAL1,   CC_FSCAL1_VALUE);     // Frequency synthesizer calibration.
    WriteRegister(CC_FSCAL0,   CC_FSCAL0_VALUE);     // Frequency synthesizer calibration.
    WriteRegister(CC_FSTEST,   CC_FSTEST_VALUE);     // Frequency synthesizer calibration.
    WriteRegister(CC_TEST2,    CC_TEST2_VALUE);      // Various test settings.
    WriteRegister(CC_TEST1,    CC_TEST1_VALUE);      // Various test settings.
    WriteRegister(CC_TEST0,    CC_TEST0_VALUE);      // Various test settings.
    WriteRegister(CC_FIFOTHR,  CC_FIFOTHR_VALUE);    // fifo threshold
    WriteRegister(CC_IOCFG2,   CC_IOCFG2_VALUE);     // GDO2 output pin configuration.
    WriteRegister(CC_IOCFG0,   CC_IOCFG0_VALUE);     // GDO0 output pin configuration.
    WriteRegister(CC_PKTCTRL1, CC_PKTCTRL1_VALUE);   // Packet automation control.
    WriteRegister(CC_PKTCTRL0, CC_PKTCTRL0_VALUE);   // Packet automation control.
    WriteRegister(CC_PKTLEN,   7);                   // Packet length - dummy

    WriteRegister(CC_PATABLE,  CC_PATABLE_INITIAL);

    WriteRegister(CC_MCSM2, CC_MCSM2_VALUE);
    WriteRegister(CC_MCSM1, CC_MCSM1_VALUE);
}

// ================================= Interrupt ================================
void cc2500_t::IHandleAsync() {
    if(State == ccTransmitting) {
        State = ccIdle;
//        chEvtBroadcastI(&IEvtSrcTx);
    }
    else if(State == ccReceiving) {
        State = ccIdle;
//        chEvtBroadcastI(&IEvtSrcRx);
    }
}

void cc2500_t::IGdo0IrqHandler() {
    IGdo0.CleanIrqFlag();
    // Resume thread if any
    chSysLockFromIsr();
    if(PWaitingThread != NULL) {
        if(PWaitingThread->p_state == THD_STATE_SUSPENDED) {
            PWaitingThread->p_u.rdymsg = RDY_OK;    // Signal that IRQ fired
            chSchReadyI(PWaitingThread);
        }
        PWaitingThread = NULL;
    }
    else IHandleAsync(); // Async task completed
    chSysUnlockFromIsr();
}

extern "C" {
    CH_IRQ_HANDLER(GDO0_IRQ_HANDLER) {
        CH_IRQ_PROLOGUE();
        CC.IGdo0IrqHandler();
        CH_IRQ_EPILOGUE();
    }
} // extern c
