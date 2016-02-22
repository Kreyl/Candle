/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

#if 1 // ========================== Packet =====================================
struct rPkt_t {
    uint8_t AppID;
    uint8_t R, G, B;
} __attribute__ ((__packed__));
#define RPKT_LEN    sizeof(rPkt_t)
#endif

#define R_APP_ID            0x09
#define RCHANNEL            4
#define RSSI_MIN_DBM        (-63)

#if 1 // =========================== Timings ===================================
#define RX_DURATION_MS      7
#define SLEEP_DURATION_MS   450

#endif

#endif /* RLVL1_DEFINS_H_ */
