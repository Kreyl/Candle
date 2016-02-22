/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#ifndef RADIO_LVL1_H_
#define RADIO_LVL1_H_

#include "ch.h"
#include "rlvl1_defins.h"
#include "kl_lib_f100.h"
#include "cc2500.h"

class rLevel1_t {
private:
    rPkt_t PktRx, PktTx;
public:
    void Init();
    // Inner use
    void ITask();
};

extern rLevel1_t Radio;

#endif /* RADIO_LVL1_H_ */
