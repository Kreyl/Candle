/*
 * File:   cc2500.h
 * Author: Laurelindo
 *
 * Created on 11 Декабрь 2009 г., 2:08
 */

#ifndef _CC2500_H
#define	_CC2500_H

/*
 * Low-level operations are here.
 * Pkt level at top side, and SPI at bottom.
 */

#include "ch.h"
#include "hal.h"
#include "cc2500defins.h"
#include "cc2500_rf_settings.h"
#include "kl_lib_f100.h"

// Pins
#define CC_GPIO     GPIOA
//#define CC_GDO2     3
#define CC_GDO0     4
#define CC_SCK      5
#define CC_MISO     6
#define CC_MOSI     7
#define CC_CS       3

// SPI
#define CC_SPI      SPI1

// ============================ Types & variables ==============================
struct rPkt_t {
    uint8_t Indx;
    int8_t RSSI;        // Received signal level, RX use only
} __attribute__ ((__packed__));
#define RPKT_LEN    (sizeof(rPkt_t)-1)  // Exclude RSSI


enum CCState_t {ccIdle, ccSleeping, ccReceiving, ccTransmitting};

class CC_t {
private:
    uint8_t IState; // Inner CC state, returned as first byte
    EventSource IEvtSrcTx, IEvtSrcRx;
    // Pins
    void CsHi() { PinSet(CC_GPIO, CC_CS); }
    void CsLo() { PinClear(CC_GPIO, CC_CS); }
    bool GDO0IsHi() { return PinIsSet(CC_GPIO, CC_GDO0); }
#ifdef CC_GDO2
    bool GDO2IsHi() { return PinIsSet(CC_GPIO, CC_GDO2); }
#endif
    void BusyWait() { while(PinIsSet(CC_GPIO, CC_MISO)); }
    // General
    uint8_t ReadWriteByte(uint8_t AByte);
    void RfConfig(void);
    int8_t RSSI_dBm(uint8_t ARawRSSI);
    // Registers and buffers
    void WriteRegister (const uint8_t Addr, const uint8_t AData);
    uint8_t ReadRegister (const uint8_t Addr);
    void WriteStrobe(uint8_t AStrobe);
    void WriteTX(uint8_t* Ptr, uint8_t Length);
    // Strobes
    void CReset()      { WriteStrobe(CC_SRES); }
    void EnterTX()     { WriteStrobe(CC_STX);  }
    void FlushTxFIFO() { WriteStrobe(CC_SFTX); }
    void GetState()    { WriteStrobe(CC_SNOP); }
    void EnterRX()     { WriteStrobe(CC_SRX);  }
    void FlushRxFIFO() { WriteStrobe(CC_SFRX); }
    void Recalibrate() {
        while (IState != CC_STB_IDLE) EnterIdle();
        WriteStrobe(CC_SCAL);
    }
public:
    CCState_t State;
    void Init();
    void SetChannel(uint8_t AChannel);
    void SetTxPower(uint8_t APwr) { WriteRegister(CC_PATABLE, APwr); }
    void SetAddress(uint8_t AAddr) { WriteRegister(CC_ADDR, AAddr); }
    // Events
    void RegisterEvtRx(EventListener *PEvtLstnr, uint8_t EvtMask) { chEvtRegisterMask(&IEvtSrcRx, PEvtLstnr, EvtMask); }
    void RegisterEvtTx(EventListener *PEvtLstnr, uint8_t EvtMask) { chEvtRegisterMask(&IEvtSrcTx, PEvtLstnr, EvtMask); }
    // State change
    void TransmitSync(rPkt_t *pPkt);
    uint8_t ReceiveSync(uint32_t Timeout_ms, rPkt_t *pPkt);
    void TransmitAsync(rPkt_t *pPkt);
    void ReceiveAsync();
    void EnterIdle()  { WriteStrobe(CC_SIDLE); }
    void Sleep() { WriteStrobe(CC_SPWD); }
    // Inner use
    inline void IHandleAsync();
    uint8_t ReadFIFO(rPkt_t *pPkt);
};

extern CC_t CC;

#endif	/* _CC2500_H */

