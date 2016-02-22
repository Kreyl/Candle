/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_f100.h"
#include "ch.h"
#include "evt_mask.h"
#include "color.h"

#if 1 // ==== Timings ====

#endif

// ==== Application class ====
class App_t {
private:
    Color_t IClr;
public:
    Thread *PThd;
    void Init();
    void SendEvtRx(uint8_t R, uint8_t G, uint8_t B) {
        IClr.Red = R;
        IClr.Green = G;
        IClr.Blue = B;
        chEvtSignal(PThd, EVTMSK_RX);
    }
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
