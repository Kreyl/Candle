/*
 * adc_f0.h
 *
 *  Created on: 16.02.2013
 *      Author: kreyl
 */

#ifndef ADC_F100_H_
#define ADC_F100_H_

#include "stm32f10x.h"

#define ADC_SAMPLE_POWER    3
#define ADC_BUF_SZ          (1<<ADC_SAMPLE_POWER)

#define ADC_DMA_STREAM      STM32_DMA1_STREAM1

class Adc_t {
private:
    uint16_t Buf[ADC_BUF_SZ];
public:
    void StartConversion() {
        ADC1->CR2 |= ADC_CR2_SWSTART;
        while(ADC1->CR2 & ADC_CR2_SWSTART);
        ADC1->CR2 |= ADC_CR2_ADON;
    }
    void Init();
    void SetChannelCount(uint32_t Count);
    void ChannelConfig(uint32_t Channel, uint32_t SampleTime, uint32_t SeqIndx);
    void ContModeEnable() { ADC1->CR2 |= ADC_CR2_CONT; }  // Enable continuos conversion
    void Enable() { ADC1->CR2 |= ADC_CR2_ADON; }
    void Disable() { ADC1->CR2 &= ~ADC_CR2_ADON; }
    void Calibrate();
    bool ConversionCompleted() { return (ADC1->SR & ADC_SR_EOC); }
    uint16_t Result() { return ADC1->DR; }
};

// ADC sampling_times
#define ADC_SampleTime_1_5Cycles                     ((uint32_t)0x00000000)
#define ADC_SampleTime_7_5Cycles                     ((uint32_t)0x00000001)
#define ADC_SampleTime_13_5Cycles                    ((uint32_t)0x00000002)
#define ADC_SampleTime_28_5Cycles                    ((uint32_t)0x00000003)
#define ADC_SampleTime_41_5Cycles                    ((uint32_t)0x00000004)
#define ADC_SampleTime_55_5Cycles                    ((uint32_t)0x00000005)
#define ADC_SampleTime_71_5Cycles                    ((uint32_t)0x00000006)
#define ADC_SampleTime_239_5Cycles                   ((uint32_t)0x00000007)


#endif /* ADC_F100_H_ */
