// ============================================================================
//  CardioVest / CardioCore V1 - ADS1298 driver (raw acquisition only)
// ----------------------------------------------------------------------------
//  Minimal SPI driver for the TI ADS1298 8-channel, 24-bit ECG AFE.
//  It only performs register I/O and raw frame reads. It does NOT, and is not
//  intended to, interpret, classify, or diagnose any signal.
//
//  Register/command values follow the ADS1298 datasheet (TI SBAS459).
//  Configuration values written by configureDefault() are CANDIDATES and must
//  be verified against the datasheet before relying on them.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "config.h"

namespace ads {

// --- SPI system commands ----------------------------------------------------
constexpr uint8_t CMD_WAKEUP  = 0x02;
constexpr uint8_t CMD_STANDBY = 0x04;
constexpr uint8_t CMD_RESET   = 0x06;
constexpr uint8_t CMD_START   = 0x08;
constexpr uint8_t CMD_STOP    = 0x0A;
// --- Data read commands -----------------------------------------------------
constexpr uint8_t CMD_RDATAC  = 0x10;  // read data continuous
constexpr uint8_t CMD_SDATAC  = 0x11;  // stop read data continuous
constexpr uint8_t CMD_RDATA   = 0x12;  // read data by command
// --- Register access (OR the register address into the low bits) ------------
constexpr uint8_t CMD_RREG    = 0x20;
constexpr uint8_t CMD_WREG    = 0x40;

// --- Register addresses (subset) --------------------------------------------
// The ADS129x family shares the command set but NOT the register map, so the
// addresses below switch on the selected AFE (see config.h).
constexpr uint8_t REG_ID      = 0x00;
constexpr uint8_t REG_CONFIG1 = 0x01;
constexpr uint8_t REG_CONFIG2 = 0x02;
#if AFE_TYPE == AFE_ADS1292R
// ADS1292R map (TI SBAS502)
constexpr uint8_t REG_LOFF    = 0x03;
constexpr uint8_t REG_CH1SET  = 0x04;  // CH1SET, CH2SET = 0x04, 0x05
constexpr uint8_t REG_RLD_SENS= 0x06;
constexpr uint8_t REG_RESP1   = 0x09;
constexpr uint8_t REG_RESP2   = 0x0A;
#else
// ADS1298 map (TI SBAS459)
constexpr uint8_t REG_CONFIG3   = 0x03;
constexpr uint8_t REG_LOFF      = 0x04;
constexpr uint8_t REG_CH1SET    = 0x05;  // CH1SET..CH8SET = 0x05..0x0C
constexpr uint8_t REG_RLD_SENSP = 0x0D;
constexpr uint8_t REG_RLD_SENSN = 0x0E;
constexpr uint8_t REG_CONFIG4   = 0x17;
#endif

}  // namespace ads

class Ads1298 {
 public:
  // Configure GPIO + SPI, pulse RESET, leave the device in SDATAC. Returns
  // true if the ID register reads back a plausible ADS129x value.
  bool begin();

  void    reset();                       // hardware reset pulse via RESET pin
  void    sendCommand(uint8_t cmd);
  void    writeReg(uint8_t reg, uint8_t value);
  uint8_t readReg(uint8_t reg);
  uint8_t readId();                      // ID register (0x00)
  static bool idLooksValid(uint8_t id);  // crude ADS129x family check

  // Write a candidate default configuration (CONFIG1/2/3, CHnSET, RLD).
  // Values are TBD and must be verified against the datasheet.
  void configureDefault();

  void startConversions();               // RDATAC + START (pin/command)
  void stopConversions();                // STOP + SDATAC

  // If DRDY is asserted (low), read one FRAME_BYTES frame into `out` and
  // return true; otherwise return false. RAW bytes only.
  bool readFrame(uint8_t* out);

 private:
  SPISettings spi_{4000000, MSBFIRST, SPI_MODE1};  // ADS1298 is SPI mode 1
  inline void csLow();
  inline void csHigh();
};
