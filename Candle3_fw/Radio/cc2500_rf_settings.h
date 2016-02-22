/*
 * File:   cc_rf_settings.h
 * Author: Kreyl Laurelindo
 *
 * Created on 7 ÐœÐ°Ñ€Ñ‚ 2010 Ð³., 12:42
 */

#ifndef _CC_RF_SETTINGS_H
#define	_CC_RF_SETTINGS_H

// Bitrate
//#define CC_BITRATE_2K4
//#define CC_BITRATE_10K
#define CC_BITRATE_250K

// =================================== Power ===================================
enum ccPwr_t {
    ccPwrMinus30dBm = 0x50,
    ccPwrMinus28dBm = 0x44,
    ccPwrMinus8dBm  = 0x6E,
    ccPwrMinus6dBm  = 0x7F,
    ccPwrMinus4dBm  = 0xA9,
    ccPwrMinus2dBm  = 0xBB,
    ccPwr0dBm       = 0xFE,
    ccPwrPlus1dBm   = 0xFF,
};

#define CC_PATABLE_INITIAL  ccPwr0dBm

// ================================= Frequency =================================
// 2433 MHz, 27MHz Crystal
#define CC_FREQ2_VALUE      0x5A        // Frequency control word, high byte.
#define CC_FREQ1_VALUE      0x1C        // Frequency control word, middle byte.
#define CC_FREQ0_VALUE      0x71        // Frequency control word, low byte.

// =================================== Common ==================================
//#define CC_MDMCFG1_VALUE    0b10100010  // FEC=1, Preamble length=010 => 4bytes, +Channel spacing
//#define CC_MDMCFG1_VALUE    0b10100011  // FEC=1, Preamble length=010 => 4bytes, +Channel spacing=11=MAX
#define CC_MDMCFG1_VALUE    0b00100010  // FEC=0, Preamble length=010 => 4bytes, +Channel spacing

//#define CC_MCSM1_VALUE      0b00001100  // Channel is always clear, RX->RX,   TX->IDLE
//#define CC_MCSM1_VALUE      0b00001111  // Channel is always clear, RX->RX,   TX->RX
#define CC_MCSM1_VALUE      0b00000000  // Channel is always clear, RX->IDLE, TX->IDLE
//#define CC_MCSM1_VALUE      0b00000011  // Channel is always clear, RX->IDLE, TX->RX
#define CC_MCSM0_VALUE      0x18        // Calibrate at IDLE->RX,TX

#define CC_FIFOTHR_VALUE    0b00000111  // RXFIFO and TXFIFO thresholds: TX 33, RX 32
#define CC_IOCFG2_VALUE     0x0E        // GDO2 - Carrier sence
//#define CC_IOCFG0_VALUE     0x0E        // GDO0 - Carrier sence
#define CC_IOCFG0_VALUE     0x06        // GDO0 - Asserts when sync word has been sent / received, and de-asserts at the end of the packet.
                                        // In RX, the pin will de-assert when the optional address check fails or the RX FIFO overflows.
//#define CC_IOCFG0_VALUE     0x01        // GDO0 - Associated to the RX FIFO: Asserts when RX FIFO is filled at or above the RX FIFO threshold
                                        // or the end of packet is reached. De-asserts when RX FIFO is drained below the same threshold.

#define CC_PKTCTRL1_VALUE   0b00001100  // PQT=0, CRC autoflush=1, Append=1, Address check = 00 (no check)
//#define CC_PKTCTRL1_VALUE   0b00001101  // PQT=0, CRC autoflush=1, Append=1, Address check = 01 (check, no broadcast)
//#define CC_PKTCTRL1_VALUE   0b00000101  // PQT=0, CRC autoflush=0, Append=1, Address check = 01 (check, no broadcast)
//#define CC_PKTCTRL1_VALUE   0b00001110  // PQT=0, CRC autoflush=1, Append=1, Address check = 10 (check, 0 is broadcast)
#define CC_PKTCTRL0_VALUE   0b01000100  // WhiteData=1, PKTFormat=normal, CC2400_EN=0, CRC enabled, Fixed Length
//#define CC_PKTCTRL0_VALUE   0b00000100  // WhiteData=0, PKTFormat=normal, CC2400_EN=0, CRC enabled, Fixed Length

// ========================= Bitrate-specific ==================================
#ifdef CC_BITRATE_10K
#define CC_FSCTRL1_VALUE    0x06        // Frequency synthesizer control: IF - RF studio
#define CC_FSCTRL0_VALUE    0x00        // Frequency synthesizer control: freq offset

#define CC_MDMCFG4_VALUE    0x78        // Modem configuration }
#define CC_MDMCFG3_VALUE    0x84        // Modem configuration } frequencies
#define CC_MDMCFG2_VALUE    0x03        // Filter, modulation=2FSK, no Manchester coding, SYNC_MODE=011 => 30/32 sync word bits
//#define CC_MDMCFG2_VALUE    0x13        // Filter, modulation=GFSK, no Manchester coding, SYNC_MODE=011 => 30/32 sync word bits
#define CC_MDMCFG0_VALUE    0xE5        // Modem configuration: channel spacing

#define CC_DEVIATN_VALUE    0x43        // Deviation: 36kHz @ 27MHz crystal

#define CC_FREND1_VALUE     0x56        // Front end RX configuration - RF studio
#define CC_FREND0_VALUE     0x10        // Front end TX configuration.

#define CC_MCSM2_VALUE      0b00000111

#define CC_FOCCFG_VALUE     0x16        // Frequency Offset Compensation Configuration - RF studio
// Bit synchronization Configuration
#define CC_BSCFG_VALUE      0b01101100  // ±0% data rate offset
//#define CC_BSCFG_VALUE      0b01101101  // ±3.125% data rate offset
//#define CC_BSCFG_VALUE      0b01101110  // ±6.25% data rate offset
//#define CC_BSCFG_VALUE      0b01101111  // ±12.5% data rate offset
#define CC_AGCCTRL2_VALUE   0x43        // }
#define CC_AGCCTRL1_VALUE   0x40        // }
#define CC_AGCCTRL0_VALUE   0x91        // } AGC control: RF studio

#define CC_FSCAL3_VALUE     0xA9        // }
#define CC_FSCAL2_VALUE     0x0A        // }
#define CC_FSCAL1_VALUE     0x00        // }
#define CC_FSCAL0_VALUE     0x11        // } Frequency synthesizer calibration: RF studio

#define CC_FSTEST_VALUE     0x59        // Frequency synthesizer calibration: RF studio
#define CC_TEST2_VALUE      0x88        // Various test settings: RF studio
#define CC_TEST1_VALUE      0x31        // Various test settings: RF studio
#define CC_TEST0_VALUE      0x0B        // Various test settings: RF studio

#endif
// ================================ 2K4 ========================================
#ifdef CC_BITRATE_2K4
#define CC_FSCTRL1_VALUE    0x08        // Frequency synthesizer control: IF - RF studio
#define CC_FSCTRL0_VALUE    0x00        // Frequency synthesizer control: freq offset

#define CC_MDMCFG4_VALUE    0x86        // Modem configuration: channel bandwidth
#define CC_MDMCFG3_VALUE    0x83        // Modem configuration.
#define CC_MDMCFG2_VALUE    0x03        // Filter, modulation format, Manchester coding, SYNC_MODE=011 => 30/32 sync word bits
#define CC_MDMCFG0_VALUE    0xF8        // Modem configuration.

#define CC_DEVIATN_VALUE    0x44        // Modem deviation setting - RF studio
#define CC_FREND1_VALUE     0x56        // Front end RX configuration - RF studio
#define CC_FREND0_VALUE     0x10        // Front end TX configuration.

#define CC_MCSM2_VALUE      0b00000111

#define CC_FOCCFG_VALUE     0x16        // Frequency Offset Compensation Configuration - RF studio
#define CC_BSCFG_VALUE      0x6C        // Bit synchronization Configuration - RF studio
#define CC_AGCCTRL2_VALUE   0x03        // }
#define CC_AGCCTRL1_VALUE   0x40        // }
#define CC_AGCCTRL0_VALUE   0x91        // } AGC control: RF studio

#define CC_FSCAL3_VALUE     0xA9        // }
#define CC_FSCAL2_VALUE     0x0A        // }
#define CC_FSCAL1_VALUE     0x00        // }
#define CC_FSCAL0_VALUE     0x11        // } Frequency synthesizer calibration: RF studio

#define CC_FSTEST_VALUE     0x59        // Frequency synthesizer calibration: RF studio
#define CC_TEST2_VALUE      0x88        // Various test settings: RF studio
#define CC_TEST1_VALUE      0x31        // Various test settings: RF studio
#define CC_TEST0_VALUE      0x0B        // Various test settings: RF studio

#endif
// ============================== 250K =========================================
#ifdef CC_BITRATE_250K
#define CC_FSCTRL1_VALUE    0x12        // Frequency synthesizer control: IF - RF studio
#define CC_FSCTRL0_VALUE    0x00        // Frequency synthesizer control: freq offset

#define CC_MDMCFG4_VALUE    0x2D        // Modem configuration: channel bandwidth
#define CC_MDMCFG3_VALUE    0x2F        // Modem configuration.
#define CC_MDMCFG2_VALUE    0x73        // Filter, modulation format, Manchester coding, SYNC_MODE=011 => 30/32 sync word bits
#define CC_MDMCFG0_VALUE    0xE5        // Modem configuration.

#define CC_DEVIATN_VALUE    0x00        // Modem deviation setting - RF studio
#define CC_FREND1_VALUE     0xB6        // Front end RX configuration - RF studio
#define CC_FREND0_VALUE     0x10        // Front end TX configuration.

#define CC_MCSM2_VALUE      0b00000111

#define CC_FOCCFG_VALUE     0x1D        // Frequency Offset Compensation Configuration - RF studio
#define CC_BSCFG_VALUE      0x1C        // Bit synchronization Configuration - RF studio
#define CC_AGCCTRL2_VALUE   0xC7        // }
#define CC_AGCCTRL1_VALUE   0x00        // }
#define CC_AGCCTRL0_VALUE   0xB0        // } AGC control: RF studio

#define CC_FSCAL3_VALUE     0xEA        // }
#define CC_FSCAL2_VALUE     0x0A        // }
#define CC_FSCAL1_VALUE     0x00        // }
#define CC_FSCAL0_VALUE     0x11        // } Frequency synthesizer calibration: RF studio

#define CC_FSTEST_VALUE     0x59        // Frequency synthesizer calibration: RF studio
#define CC_TEST2_VALUE      0x88        // Various test settings: RF studio
#define CC_TEST1_VALUE      0x31        // Various test settings: RF studio
#define CC_TEST0_VALUE      0x0B        // Various test settings: RF studio

#endif

// ********************
// Rare use settings
#define CC_SYNC1_VALUE      0xD3    // }
#define CC_SYNC0_VALUE      0x91    // } SYNC word to transmit


#endif	/* _CC_RF_SETTINGS_H */

