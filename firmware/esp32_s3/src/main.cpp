// ============================================================================
//  CardioVest  -  CardioCore V1
//  ESP32-S3 firmware  -  main.cpp
// ----------------------------------------------------------------------------
//  Project : CardioVest (wearable multi-channel ECG acquisition platform)
//  Module  : CardioCore V1 (8-channel, 24-bit ECG acquisition board)
//  Target  : ESP32-S3-WROOM-1-N16R8 (16 MB flash / 8 MB PSRAM)
//  AFE     : Texas Instruments ADS1298 (8-ch, 24-bit, simultaneous sampling)
//
//  >>> RESEARCH, PROTOTYPING, AND EDUCATION ONLY <<<
//
//  This firmware and the CardioCore V1 hardware are NOT a medical device. They
//  are NOT certified, approved, or cleared by any regulatory body. This code
//  does NOT diagnose, treat, monitor patients, or make any clinical decision,
//  and does NOT interpret ECG signals clinically. It only acquires, buffers,
//  streams, and logs RAW analog-front-end samples for engineering research and
//  educational experimentation.
//
//  Pipeline:  ADS1298 (DRDY) -> ring buffer -> { BLE notify, microSD log }
//  Safety  :  acquisition is gated by interlock::acquisitionPermitted()
//             (battery-only during body-connected measurement; see Safety doc).
// ============================================================================
#include <Arduino.h>
#include <SPI.h>

#include "config.h"
#include "pins.h"
#include "ring_buffer.h"
#include "ads1298.h"
#include "ble_stream.h"
#include "sd_log.h"
#include "interlock.h"

namespace {

Ads1298 g_ads;
FrameRing<cfg::FRAME_BYTES, cfg::RING_FRAMES> g_ring;
volatile bool g_drdy = false;   // set by the DRDY ISR
bool g_acq = false;             // conversions currently running?

void IRAM_ATTR onDrdy() { g_drdy = true; }

void printBanner() {
  Serial.println();
  Serial.println(F("============================================================"));
  Serial.print  (F("  ")); Serial.println(F(FW_PROJECT));
  Serial.print  (F("  Firmware: ")); Serial.println(F(FW_VERSION));
  Serial.print  (F("  AFE: ")); Serial.print(cfg::AFE_NAME);
  Serial.print  (F("  |  Channels: "));
  Serial.print  (cfg::NUM_CHANNELS);
  Serial.print  (F(" x 24-bit  |  Frame: "));
  Serial.print  (cfg::FRAME_BYTES); Serial.println(F(" bytes"));
  Serial.println(F("------------------------------------------------------------"));
  Serial.println(F("  RESEARCH / PROTOTYPING / EDUCATION ONLY - NOT a medical"));
  Serial.println(F("  device. No diagnosis, treatment, monitoring, or emergency"));
  Serial.println(F("  use. Raw data only; no clinical interpretation."));
  Serial.println(F("============================================================"));
  Serial.println();
}

}  // namespace

void setup() {
  Serial.begin(cfg::SERIAL_BAUD);
  const uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) { /* bounded wait for USB-CDC host */ }
  printBanner();

  interlock::begin();

  Serial.printf("[init] %s...\n", cfg::AFE_NAME);
  if (g_ads.begin()) {
    Serial.printf("[init] %s detected, ID=0x%02X\n", cfg::AFE_NAME, g_ads.readId());
    g_ads.configureDefault();
  } else {
    Serial.printf("[init] %s NOT detected - check wiring and power.\n", cfg::AFE_NAME);
    Serial.println(F("[init] (ADS1298 board: also check the CLKSEL strap - finding F1.)"));
  }

  attachInterrupt(digitalPinToInterrupt(pins::ADS_DRDY), onDrdy, FALLING);

  if (cfg::ENABLE_BLE) {
    Serial.println(F("[init] BLE (NimBLE)..."));
    ble_stream::begin("CardioCore-V1");
  }

  if (cfg::ENABLE_SD) {
    Serial.println(F("[init] microSD..."));
    if (sd_log::begin() && sd_log::startSession()) {
      Serial.println(F("[init] microSD session open."));
    } else {
      Serial.println(F("[init] microSD not available (continuing without logging)."));
    }
  }

  // Start acquisition only if the safety interlock permits it.
  if (interlock::acquisitionPermitted()) {
    interlock::setAfeEnabled(true);
    g_ads.startConversions();
    g_acq = true;
    Serial.println(F("[init] Acquisition started."));
  } else {
    Serial.println(F("[init] Acquisition INHIBITED by interlock (USB present)."));
  }

  Serial.println(F("[init] Setup complete."));
}

void loop() {
  static uint32_t lastBeat = 0, lastFlush = 0;
  const uint32_t now = millis();

  // --- Safety interlock: stop acquiring if USB power appears ----------------
  if (g_acq && !interlock::acquisitionPermitted()) {
    g_ads.stopConversions();
    interlock::setAfeEnabled(false);
    g_acq = false;
    Serial.println(F("[interlock] Acquisition stopped (USB present)."));
  }

  // --- Producer: drain the ADS1298 on data-ready into the ring buffer -------
  if (g_acq && g_drdy) {
    g_drdy = false;
    uint8_t frame[cfg::FRAME_BYTES];
    if (g_ads.readFrame(frame)) g_ring.push(frame);
  }

  // --- Consumer: forward buffered RAW frames to the sinks -------------------
  uint8_t out[cfg::FRAME_BYTES];
  while (g_ring.pop(out)) {
    if (cfg::ENABLE_BLE) ble_stream::sendFrame(out, cfg::FRAME_BYTES);
    if (cfg::ENABLE_SD)  sd_log::writeFrame(out, cfg::FRAME_BYTES);
  }

  // --- Periodic SD flush ----------------------------------------------------
  if (cfg::ENABLE_SD && (now - lastFlush >= cfg::SD_FLUSH_MS)) {
    lastFlush = now;
    sd_log::flush();
  }

  // --- Heartbeat ------------------------------------------------------------
  if (now - lastBeat >= cfg::HEARTBEAT_MS) {
    lastBeat = now;
    Serial.printf("[status] up=%lus | acq=%d | ble=%d | drops=%lu | research/edu only\n",
                  static_cast<unsigned long>(now / 1000), static_cast<int>(g_acq),
                  static_cast<int>(ble_stream::connected()),
                  static_cast<unsigned long>(g_ring.dropped()));
  }
}
