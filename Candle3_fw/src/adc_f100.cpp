/*
 * adc_f0.cpp
 *
 *  Created on: 16.02.2013
 *      Author: kreyl
 */

#include "adc_f100.h"
#include "hal.h"
#include "kl_lib_f100.h"
#include "clocking_f100.h"

void Adc_t::Init() {
    Clk.SetupAdcClk(adcDiv4);
    rccEnableADC1(FALSE);
    PinSetupAnalog(GPIOB, 1);

    Enable();       // May configure only enabled ADC
    SetChannelCount(1);
    ChannelConfig(9, ADC_SampleTime_55_5Cycles, 1);
    Calibrate();    // Must be performed when ADC is disabled
}

void Adc_t::SetChannelCount(uint32_t Count) {
    ADC1->SQR1 &= ~ADC_SQR1_L;  // Clear count
    ADC1->SQR1 |= (Count-1) << 20;
}

void Adc_t::ChannelConfig(uint32_t Channel, uint32_t SampleTime, uint32_t SeqIndx) {
    if(Channel <= 9) {
        uint32_t Offset = Channel * 3;
        ADC1->SMPR2 &= ~((uint32_t)0b111 << Offset);
        ADC1->SMPR2 |= SampleTime << Offset;
    }
    else {
        uint32_t Offset = (Channel - 10) * 3;
        ADC1->SMPR1 &= ~((uint32_t)0b111 << Offset);
        ADC1->SMPR1 |= SampleTime << Offset;
    }
    // Put channel number in appropriate place in sequence
    if(SeqIndx <= 6) {
        uint32_t Offset = (SeqIndx-1)*5;
        ADC1->SQR3 &= ~(uint32_t)(0b11111 << Offset);
        ADC1->SQR3 |= (uint32_t)(Channel << Offset);
    }
    else if(SeqIndx <= 12) {
        uint32_t Offset = (SeqIndx-7)*5;
        ADC1->SQR2 &= ~(uint32_t)(0b11111 << Offset);
        ADC1->SQR2 |= (uint32_t)(Channel << Offset);
    }
    else if(SeqIndx <= 16) {
        uint32_t Offset = (SeqIndx-13)*5;
        ADC1->SQR1 &= ~(uint32_t)(0b11111 << Offset);
        ADC1->SQR1 |= (uint32_t)(Channel << Offset);
    }
}

void Adc_t::Calibrate() {
    ADC1->CR2 |= ADC_CR2_RSTCAL;        // Reset calibration
    while(ADC1->CR2 & ADC_CR2_RSTCAL);  // Wait until reset completed
    ADC1->CR2 |= ADC_CR2_CAL;           // Start calibration
    while(ADC1->CR2 & ADC_CR2_CAL);     // Wait until calibration completed
}
