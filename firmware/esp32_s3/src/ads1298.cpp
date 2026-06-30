// ============================================================================
//  CardioVest / CardioCore V1 - ADS1298 driver implementation
//  Raw register I/O + frame reads only. No signal interpretation.
//  Configuration values are CANDIDATES - verify against TI SBAS459.
// ============================================================================
#include "ads1298.h"
#include "pins.h"

inline void Ads1298::csLow()  { digitalWrite(pins::ADS_CS, LOW);  }
inline void Ads1298::csHigh() { digitalWrite(pins::ADS_CS, HIGH); }

bool Ads1298::begin() {
  pinMode(pins::ADS_CS,    OUTPUT); digitalWrite(pins::ADS_CS,    HIGH);
  pinMode(pins::ADS_RESET, OUTPUT); digitalWrite(pins::ADS_RESET, HIGH);
  pinMode(pins::ADS_PWDN,  OUTPUT); digitalWrite(pins::ADS_PWDN,  HIGH);
  pinMode(pins::ADS_START, OUTPUT); digitalWrite(pins::ADS_START, LOW);
  pinMode(pins::ADS_DRDY,  INPUT);

  SPI.begin(pins::ADS_SCLK, pins::ADS_MISO, pins::ADS_MOSI, pins::ADS_CS);

  delay(20);                       // let supplies / PWDN settle
  reset();
  sendCommand(ads::CMD_SDATAC);    // leave continuous mode so registers read
  delay(1);

  return idLooksValid(readId());
}

void Ads1298::reset() {
  digitalWrite(pins::ADS_RESET, HIGH); delay(1);
  digitalWrite(pins::ADS_RESET, LOW);  delayMicroseconds(10);  // >= 2 t_CLK
  digitalWrite(pins::ADS_RESET, HIGH); delay(1);               // >= 18 t_CLK after reset
}

void Ads1298::sendCommand(uint8_t cmd) {
  SPI.beginTransaction(spi_);
  csLow();
  SPI.transfer(cmd);
  csHigh();
  SPI.endTransaction();
}

void Ads1298::writeReg(uint8_t reg, uint8_t value) {
  SPI.beginTransaction(spi_);
  csLow();
  SPI.transfer(ads::CMD_WREG | reg);
  SPI.transfer(0x00);              // write count - 1 == 0 (one register)
  SPI.transfer(value);
  csHigh();
  SPI.endTransaction();
}

uint8_t Ads1298::readReg(uint8_t reg) {
  SPI.beginTransaction(spi_);
  csLow();
  SPI.transfer(ads::CMD_RREG | reg);
  SPI.transfer(0x00);              // read count - 1 == 0 (one register)
  const uint8_t v = SPI.transfer(0x00);
  csHigh();
  SPI.endTransaction();
  return v;
}

uint8_t Ads1298::readId() { return readReg(ads::REG_ID); }

bool Ads1298::idLooksValid(uint8_t id) {
  // Crude sanity check on the ID register - confirm the exact value on hardware.
  if (id == 0x00 || id == 0xFF) return false;
#if AFE_TYPE == AFE_ADS1292R
  // ADS1292R typically reads 0x73 (ADS1292 = 0x53). Be lenient; this only gates
  // a console warning, the firmware continues either way.
  return id == 0x73 || id == 0x53 || (id & 0x10) != 0;
#else
  // ADS1298 family typically reads ~0x92 (0x9x device class).
  return (id & 0xF8) == 0x90;
#endif
}

void Ads1298::configureDefault() {
  sendCommand(ads::CMD_SDATAC);    // must stop continuous read to write registers

#if AFE_TYPE == AFE_ADS1292R
  // ----- ADS1292R candidate config (VERIFY against TI SBAS502) --------------
  // CONFIG2 (0xA0): enable internal reference buffer (PDB_REFBUF). To enable the
  // built-in square-wave test signal during bring-up, use 0xA3 (INT_TEST|TEST_FREQ).
  writeReg(ads::REG_CONFIG2, 0xA0);
  // CONFIG1 (0x02): continuous mode, 500 SPS (DR=010). CANDIDATE - VERIFY.
  writeReg(ads::REG_CONFIG1, 0x02);
  // CH1SET/CH2SET (0x00): normal electrode input, PGA gain = 6. CANDIDATE.
  for (uint8_t ch = 0; ch < cfg::NUM_CHANNELS; ++ch) {
    writeReg(ads::REG_CH1SET + ch, 0x00);
  }
  writeReg(ads::REG_RLD_SENS, 0x00);   // RLD routing TBD
  writeReg(ads::REG_RESP1, 0x00);      // respiration measurement off
  writeReg(ads::REG_RESP2, 0x02);      // RESP2 default
#else
  // ----- ADS1298 candidate config (VERIFY against TI SBAS459) ---------------
  // CONFIG3 (0xEC): internal RLD reference, RLD buffer powered.
  //   NOTE: the board uses an EXTERNAL REF5025 on VREFP - decide whether the
  //   internal reference buffer (PD_REFBUF) should be OFF here. CANDIDATE - VERIFY (B4).
  writeReg(ads::REG_CONFIG3, 0xEC);
  // CONFIG1 (0x06): DR=500 SPS. NOTE: bit7 (HR) is 0 here = low-power mode;
  // for high-resolution use 0x86 (HR | DR). CANDIDATE - VERIFY (SBAS459).
  writeReg(ads::REG_CONFIG1, 0x06);
  // CONFIG2 (0x00): internal test signal disabled. Use 0x10|0x03 to enable the
  // built-in square-wave test signal during bring-up (Bringup_Plan stage 5).
  writeReg(ads::REG_CONFIG2, 0x00);
  // CHnSET (0x00): normal electrode input, PGA gain = 6. Per-channel gain/mux
  // is TBD and depends on the chosen montage (B8).
  for (uint8_t ch = 0; ch < cfg::NUM_CHANNELS; ++ch) {
    writeReg(ads::REG_CH1SET + ch, 0x00);
  }
  // RLD sensed-channel selection - which channels feed the right-leg drive.
  // 0x00 = none yet; configure per the final montage / RLD design (B3, B8).
  writeReg(ads::REG_RLD_SENSP, 0x00);
  writeReg(ads::REG_RLD_SENSN, 0x00);
#endif
}

void Ads1298::startConversions() {
  sendCommand(ads::CMD_RDATAC);          // continuous data mode
  digitalWrite(pins::ADS_START, HIGH);   // begin conversions
}

void Ads1298::stopConversions() {
  digitalWrite(pins::ADS_START, LOW);
  sendCommand(ads::CMD_SDATAC);
}

bool Ads1298::readFrame(uint8_t* out) {
  // Clock out one raw frame. The caller invokes this in response to the latched
  // DRDY interrupt (RDATAC mode), so we do NOT re-gate on a fresh digitalRead -
  // the brief DRDY low pulse may already have ended by the time we run.
  SPI.beginTransaction(spi_);
  csLow();
  for (uint16_t i = 0; i < cfg::FRAME_BYTES; ++i) {
    out[i] = SPI.transfer(0x00);         // clock out one raw frame
  }
  csHigh();
  SPI.endTransaction();
  return true;
}
