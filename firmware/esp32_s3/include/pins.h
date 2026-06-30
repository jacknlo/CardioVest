// ============================================================================
//  CardioVest / CardioCore V1 - ESP32-S3 (U1) pin map
// ----------------------------------------------------------------------------
//  Extracted from the flux.ai EDIF netlist:
//    hardware/CardioCore_V1/schematics/CardioCore_V1.edif
//  (see docs/AFE_Verification_Report.md). Verify against the schematic before
//  hardware bring-up.
// ============================================================================
#pragma once

namespace pins {

// --- ADS1298 (U2): SPI bus + control lines ----------------------------------
static constexpr int ADS_SCLK  = 12;   // SPI_SCLK  -> U1.IO12
static constexpr int ADS_MOSI  = 11;   // SPI_MOSI  -> U1.IO11 (ADS1298 DIN)
static constexpr int ADS_MISO  = 13;   // SPI_MISO  -> U1.IO13 (ADS1298 DOUT)
static constexpr int ADS_CS    = 10;   // ADS_CS    -> U1.IO10 (active low)
static constexpr int ADS_DRDY  = 14;   // ADS_DRDY  -> U1.IO14 (active low, IRQ in)
static constexpr int ADS_START = 15;   // ADS_START -> U1.IO15
static constexpr int ADS_RESET = 16;   // ADS_RESET -> U1.IO16 (active low)
static constexpr int ADS_PWDN  = 17;   // ADS_PWDN  -> U1.IO17 (active low)

// --- microSD (J2): shares the SPI bus ---------------------------------------
static constexpr int SD_CS     = 9;    // SD_CS     -> U1.IO9 (CLK/MOSI/MISO shared)

// --- I2C (charger MP2662 U3 + expansion) ------------------------------------
static constexpr int I2C_SDA   = 4;    // I2C_SDA   -> U1.IO4
static constexpr int I2C_SCL   = 5;    // I2C_SCL   -> U1.IO5
static constexpr int CHG_INT   = 6;    // CHG_INT   -> U1.IO6 (MP2662 INT, active low)

// --- Expansion header (J5) --------------------------------------------------
static constexpr int EXP_GPIO1 = 7;    // EXP_GPIO1 -> U1.IO7
static constexpr int EXP_GPIO2 = 8;    // EXP_GPIO2 -> U1.IO8

// --- Buttons ----------------------------------------------------------------
static constexpr int BTN_BOOT  = 0;    // ESP_BOOT  -> U1.IO0 (SW2); SW1 -> chip EN

// --- USB native (handled by the USB peripheral; for documentation) ----------
//   D- = U1.IO19, D+ = U1.IO20

// --- Safety interlock (HARDWARE NOT YET ASSIGNED: blockers B9 / B10) ---------
// USB-present sensing and AFE-enable control are not yet in the schematic.
// -1 means "not wired"; the firmware degrades gracefully and warns.
static constexpr int USB_PRESENT = -1;  // TBD (B9) - could derive from MP2662 status (I2C) or a VBUS divider
static constexpr int AFE_ENABLE  = -1;  // TBD (B10) - control that disables the AFE/electrode path on USB

}  // namespace pins
