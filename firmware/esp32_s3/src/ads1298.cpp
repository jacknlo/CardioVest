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
  // The ADS129x needs t_SDECODE (>= 4 t_CLK ~= 2 us @ f_CLK 2.048 MHz) to decode
  // each command byte. With SCLK at 4 MHz, back-to-back SPI.transfer() calls can
  // clock the next byte before the previous one is decoded -> intermittently
  // wrong register writes (classic ADS129x gotcha). Space the command bytes.
  SPI.beginTransaction(spi_);
  csLow();
  SPI.transfer(ads::CMD_WREG | reg);
  delayMicroseconds(2);            // t_SDECODE
  SPI.transfer(0x00);              // write count - 1 == 0 (one register)
  delayMicroseconds(2);            // t_SDECODE
  SPI.transfer(value);
  csHigh();
  SPI.endTransaction();
}

uint8_t Ads1298::readReg(uint8_t reg) {
  SPI.beginTransaction(spi_);
  csLow();
  SPI.transfer(ads::CMD_RREG | reg);
  delayMicroseconds(2);            // t_SDECODE (see writeReg)
  SPI.transfer(0x00);              // read count - 1 == 0 (one register)
  delayMicroseconds(2);            // t_SDECODE
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
  // ADS1298 reads ~0x92 (0x9x class); ADS1298R reads ~0xD2 (0xDx class). Accept
  // both so an ADS1298R sourcing substitution is not rejected at bring-up.
  return (id & 0xF8) == 0x90 || (id & 0xF8) == 0xD0;
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
  // CONFIG3 (0x6C): PD_REFBUF cleared (bit7=0) so the INTERNAL reference buffer
  //   is powered DOWN - the board drives VREFP from the EXTERNAL REF5025 (2.5 V),
  //   and enabling the internal buffer here would contend with it. Other bits
  //   unchanged from the previous 0xEC (RLD_MEAS, RLDREF_INT). VREF_4V is a
  //   don't-care with an external reference. CANDIDATE - VERIFY (B4).
  writeReg(ads::REG_CONFIG3, 0x6C);
  // CONFIG1 (0x86): HR (bit7=1, high-resolution) | DR=110 -> 500 SPS, matching
  //   cfg::DEFAULT_SPS. With HR=0 the same DR gives 250 SPS (half rate), which
  //   would make the host timebase wrong by 2x. CANDIDATE - VERIFY (SBAS459).
  writeReg(ads::REG_CONFIG1, 0x86);
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

  // Read back the key config registers and flag any mismatch. A silent SPI
  // decode error (t_SDECODE, wiring, contention with the SD card on the shared
  // bus) shows up here instead of as mysteriously bad data later.
  struct { uint8_t reg, want; const char* name; } checks[] = {
#if AFE_TYPE == AFE_ADS1292R
    { ads::REG_CONFIG1, 0x02, "CONFIG1" },
    { ads::REG_CONFIG2, 0xA0, "CONFIG2" },
#else
    { ads::REG_CONFIG1, 0x86, "CONFIG1" },
    { ads::REG_CONFIG2, 0x00, "CONFIG2" },
    { ads::REG_CONFIG3, 0x6C, "CONFIG3" },
#endif
  };
  for (auto& c : checks) {
    const uint8_t got = readReg(c.reg);
    if (got != c.want) {
      Serial.printf("[ads] WARN: %s read back 0x%02X, expected 0x%02X - check SPI/wiring.\n",
                    c.name, got, c.want);
    }
  }
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
