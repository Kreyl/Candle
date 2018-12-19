/*
 * board.h
 *
 *  Created on: 12 сент. 2015 г.
 *      Author: Kreyl
 */

#pragma once

#include <inttypes.h>

// ==== General ====
#define BOARD_NAME          "Candle05"
#define APP_NAME            "Candle_TxAcg"

// ==== High-level peripery control ====
#define BUTTONS_ENABLED         TRUE

#define SIMPLESENSORS_ENABLED   BUTTONS_ENABLED

// MCU type as defined in the ST header.
#define STM32L151xB

#define SYS_TIM_CLK         (Clk.APB1FreqHz)
#define I2C1_ENABLED        FALSE
#define I2C_USE_SEMAPHORE   FALSE
#define ADC_REQUIRED        FALSE

#if 1 // ========================== GPIO =======================================
// PortMinTim_t: GPIO, Pin, Tim, TimChnl, invInverted, omPushPull, TopValue
// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     2
#define UART_RX_PIN     3

// LED
#define LED_EN_PIN      { GPIOB, 2, omPushPull }
#define LED_R_PIN       { GPIOB, 5, TIM3, 2, invInverted, omOpenDrain, 255 }
#define LED_G_PIN       { GPIOB, 0, TIM3, 3, invInverted, omOpenDrain, 255 }
#define LED_B_PIN       { GPIOB, 1, TIM3, 4, invInverted, omOpenDrain, 255 }

// Buttons
#define BTN_TX_PIN      GPIOA, 0, pudPullDown
#define BTN_UP_PIN      GPIOB, 10, pudPullDown
#define BTN_DOWN_PIN    GPIOB, 11, pudPullDown

#define BTN_TX_INDX     0
#define BTN_UP_INDX     1
#define BTN_DOWN_INDX   2

// Acc
#define ACG_IRQ_PIN     GPIOC, 13
#define ACG_CS_PIN      GPIOB, 12
#define ACG_SCK_PIN     GPIOB, 13, omPushPull, pudNone, AF5
#define ACG_MISO_PIN    GPIOB, 14, omPushPull, pudNone, AF5
#define ACG_MOSI_PIN    GPIOB, 15, omPushPull, pudNone, AF5
#define ACG_PWR_PIN     GPIOA, 9
#define ACG_SPI         SPI2

// Radio: SPI, PGpio, Sck, Miso, Mosi, Cs, Gdo0
#define CC_Setup0       SPI1, GPIOA, 5,6,7, 4, 1

#endif // GPIO

#if ADC_REQUIRED // ======================= Inner ADC ==========================
// Clock divider: clock is generated from the APB2
#define ADC_CLK_DIVIDER     adcDiv2

// ADC channels
//#define BAT_CHNL          1

#define ADC_VREFINT_CHNL    17  // All 4xx, F072 and L151 devices. Do not change.
#define ADC_CHANNELS        { ADC_VREFINT_CHNL }
#define ADC_CHANNEL_CNT     1   // Do not use countof(AdcChannels) as preprocessor does not know what is countof => cannot check
#define ADC_SAMPLE_TIME     ast96Cycles
#define ADC_SAMPLE_CNT      8   // How many times to measure every channel

#define ADC_SEQ_LEN         (ADC_SAMPLE_CNT * ADC_CHANNEL_CNT)

#endif

#if 1 // =========================== DMA =======================================
#define STM32_DMA_REQUIRED  TRUE
// ==== Uart ====
#define UART_DMA_TX_MODE(Chnl) (STM32_DMA_CR_CHSEL(Chnl) | DMA_PRIORITY_LOW | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MINC | STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_TCIE)
#define UART_DMA_RX_MODE(Chnl) (STM32_DMA_CR_CHSEL(Chnl) | DMA_PRIORITY_MEDIUM | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MINC | STM32_DMA_CR_DIR_P2M | STM32_DMA_CR_CIRC)
#define UART_DMA_TX     STM32_DMA1_STREAM7
#define UART_DMA_RX     STM32_DMA1_STREAM6
#define UART_DMA_CHNL   2

// ==== ACG ====
#define ACG_DMA_TX      STM32_DMA1_STREAM5
#define ACG_DMA_RX      STM32_DMA1_STREAM4
#define ACG_DMA_CHNL    1

#if I2C1_ENABLED // ==== I2C ====
#define I2C1_DMA_TX     STM32_DMA1_STREAM6
#define I2C1_DMA_RX     STM32_DMA1_STREAM7
#endif

#if ADC_REQUIRED
#define ADC_DMA         STM32_DMA1_STREAM1
#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(0) |   /* dummy */ \
                        DMA_PRIORITY_LOW | \
                        STM32_DMA_CR_MSIZE_HWORD | \
                        STM32_DMA_CR_PSIZE_HWORD | \
                        STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */
#endif // ADC

#endif // DMA

#if 1 // ========================== USART ======================================
#define PRINTF_FLOAT_EN FALSE
#define UART_TXBUF_SZ   1024
#define UART_RXBUF_SZ   99

#define UARTS_CNT       1

#define CMD_UART_PARAMS \
    USART2, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN, \
    UART_DMA_TX, UART_DMA_RX, UART_DMA_TX_MODE(UART_DMA_CHNL), UART_DMA_RX_MODE(UART_DMA_CHNL)

#endif

