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
#include <math.h>
#include <string.h>

#include "config.h"
#include "pins.h"
#include "ring_buffer.h"
#include "ads1298.h"
#include "ble_stream.h"
#include "sd_log.h"
#include "interlock.h"

namespace {

Ads1298 g_ads;
FrameRing<cfg::TX_FRAME_BYTES, cfg::RING_FRAMES> g_ring;   // holds transport frames (seq + raw)
volatile bool g_drdy = false;   // set by the DRDY ISR
bool g_acq = false;             // conversions currently running?
uint32_t g_sampleIndex = 0;     // monotonic per-sample counter (real + demo)

void IRAM_ATTR onDrdy() { g_drdy = true; }

// --- Demo stream: synthesize ECG when no AFE is attached (dev aid) -----------
bool     g_demo = false;
uint32_t g_demoSample = 0, g_demoLastUs = 0;
bool     g_demoInit = false;   // first-call latch (micros() can legitimately be 0)
bool     g_pendMark = false;   // an event marker is waiting to be delivered over BLE
uint8_t  g_pendCount = 0;      // remaining resends
uint32_t g_markerId = 0, g_pendId = 0, g_pendIdx = 0, g_pendLastMs = 0;

inline void put24be(uint8_t* p, int32_t v) { p[0]=(v>>16)&0xFF; p[1]=(v>>8)&0xFF; p[2]=v&0xFF; }
inline void putSeqLE(uint8_t* p, uint32_t v) { p[0]=v&0xFF; p[1]=(v>>8)&0xFF; p[2]=(v>>16)&0xFF; p[3]=(v>>24)&0xFF; }

int32_t demoEcgCounts(float t, int lead) {
  const float hr = 72.0f/60.0f, ph = fmodf(t*hr, 1.0f);
  auto g = [&](float c, float w, float a){ float d=ph-c; return a*expf(-(d*d)/(2.0f*w*w)); };
  float mv = g(0.20f,0.025f,0.10f) - g(0.38f,0.008f,0.13f) + g(0.40f,0.011f,1.0f)
           - g(0.43f,0.010f,0.28f) + g(0.62f,0.040f,0.30f);
  mv *= 0.55f + 0.5f*fabsf(sinf(1.3f*lead + 0.6f));
  const float vref = 2.5f, pga = 6.0f;
  return (int32_t)lroundf(mv * 1e-3f * pga / vref * 8388607.0f);   // mV -> raw counts
}

void produceDemoFrames() {
  const uint32_t periodUs = 1000000UL / cfg::DEFAULT_SPS;
  const uint32_t now = micros();
  if (!g_demoInit) { g_demoInit = true; g_demoLastUs = now; }
  uint8_t tx[cfg::TX_FRAME_BYTES];
  int guard = 0;
  while ((uint32_t)(now - g_demoLastUs) >= periodUs && guard++ < 64) {
    g_demoLastUs += periodUs;
    const float t = (float)g_demoSample / (float)cfg::DEFAULT_SPS;
    g_demoSample++;
    putSeqLE(tx, g_sampleIndex++);
    uint8_t* raw = tx + cfg::SEQ_BYTES;
    raw[0]=raw[1]=raw[2]=0;   // status word
    for (uint8_t c=0; c<cfg::NUM_CHANNELS; ++c)
      put24be(&raw[cfg::STATUS_BYTES + c*cfg::CH_BYTES], demoEcgCounts(t, c));
    g_ring.push(tx);
  }
}

// Event-marker button (Zio-style): press the BOOT button to timestamp a moment
// of interest. Emits a marker (serial + BLE) carrying the current sample index.
void checkMarkerButton() {
  static int stable = HIGH, lastRead = HIGH;
  static uint32_t tChange = 0, markCount = 0;
  const int r = digitalRead(pins::BTN_BOOT);   // BOOT button: active low (INPUT_PULLUP)
  const uint32_t now = millis();
  if (r != lastRead) { lastRead = r; tChange = now; }
  if ((now - tChange) > 40 && r != stable) {   // 40 ms debounce
    stable = r;
    if (stable == LOW) {                        // press edge
      const uint32_t idx = g_sampleIndex;
      markCount++;
      Serial.printf("[event] marker #%lu @ sample=%lu t=%lums\n",
                    (unsigned long)markCount, (unsigned long)idx, (unsigned long)now);
      g_markerId++;                                          // deliver over BLE, resent for reliability
      g_pendId = g_markerId; g_pendIdx = idx; g_pendCount = 8; g_pendLastMs = 0; g_pendMark = true;
    }
  }
}

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
  pinMode(pins::BTN_BOOT, INPUT_PULLUP);   // event-marker button (BOOT)

  Serial.printf("[init] %s...\n", cfg::AFE_NAME);
  const bool afe = g_ads.begin();
  if (afe) {
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

  // Start real acquisition if an AFE is present and the interlock permits it;
  // otherwise fall back to the synthetic demo stream (dev aid).
  if (afe && interlock::acquisitionPermitted()) {
    interlock::setAfeEnabled(true);
    g_ads.startConversions();
    g_acq = true;
    Serial.println(F("[init] Acquisition started."));
  } else if (!afe && cfg::ENABLE_DEMO_STREAM) {
    g_demo = true;
    Serial.println(F("[init] No AFE -> DEMO STREAM: synthesizing ECG over BLE/SD"));
    Serial.println(F("[init] (test the wireless pipeline with just the ESP32-S3)."));
  } else if (afe) {
    Serial.println(F("[init] Acquisition INHIBITED by interlock (not battery-verified; see above)."));
  }

  Serial.println(F("[init] Setup complete."));
}

void loop() {
  static uint32_t lastBeat = 0, lastFlush = 0;
  const uint32_t now = millis();

  checkMarkerButton();   // BOOT-button event marker (Zio-style)

  // --- Safety interlock: stop acquiring if USB power appears ----------------
  if (g_acq && !interlock::acquisitionPermitted()) {
    g_ads.stopConversions();
    interlock::setAfeEnabled(false);
    g_acq = false;
    Serial.println(F("[interlock] Acquisition stopped (USB present)."));
  }

  // --- Producer: drain the ADS1298 on data-ready into the ring buffer -------
  // DRDY fired at least once; read that frame, then drain any further ready
  // samples (handles a loop stall spanning more than one sample period).
  if (g_acq && g_drdy) {
    uint8_t raw[cfg::FRAME_BYTES];
    uint8_t tx[cfg::TX_FRAME_BYTES];
    do {
      g_ads.readFrame(raw);
      putSeqLE(tx, g_sampleIndex++);
      memcpy(tx + cfg::SEQ_BYTES, raw, cfg::FRAME_BYTES);
      g_ring.push(tx);
    } while (digitalRead(pins::ADS_DRDY) == LOW);
    g_drdy = false;   // clear AFTER draining: an edge serviced mid-drain must not re-trigger a phantom read
  }

  // --- Demo producer: synthesize frames when no AFE is attached -------------
  if (g_demo) produceDemoFrames();

  // --- Consumer: forward buffered RAW frames to the sinks -------------------
  // Consumer: SD logs per-frame; BLE batches several frames per notification to
  // cut the notify rate (throughput / fewer drops).
  static uint8_t  batch[cfg::TX_BATCH_BYTES];
  static uint16_t batchFill = 0;
  static uint32_t lastTxMs  = 0;
  uint8_t out[cfg::TX_FRAME_BYTES];
  while (g_ring.pop(out)) {
    if (cfg::ENABLE_SD) sd_log::writeFrame(out, cfg::TX_FRAME_BYTES);
    if (cfg::ENABLE_BLE) {
      memcpy(batch + batchFill * cfg::TX_FRAME_BYTES, out, cfg::TX_FRAME_BYTES);
      if (++batchFill >= cfg::BATCH_FRAMES) {
        ble_stream::sendFrame(batch, (size_t)batchFill * cfg::TX_FRAME_BYTES);
        batchFill = 0; lastTxMs = now;
      }
    }
  }
  // Flush a partial batch if the stream slows/pauses (keeps latency bounded).
  if (cfg::ENABLE_BLE && batchFill > 0 && (now - lastTxMs) > 30) {
    ble_stream::sendFrame(batch, (size_t)batchFill * cfg::TX_FRAME_BYTES);
    batchFill = 0; lastTxMs = now;
  }

  // Deliver a pending event marker over BLE. A single notify can be dropped when
  // the data stream is busy, so resend it a few times spaced ~20 ms apart; the
  // viewer de-duplicates by marker id.
  if (g_pendMark) {
    if (!ble_stream::connected()) {
      g_pendMark = false;                                  // no client -> drop
    } else if (now - g_pendLastMs >= 20) {
      g_pendLastMs = now;
      ble_stream::sendMarker(g_pendId, g_pendIdx);
      if (--g_pendCount == 0) g_pendMark = false;
    }
  }

  // --- Periodic SD flush ----------------------------------------------------
  if (cfg::ENABLE_SD && (now - lastFlush >= cfg::SD_FLUSH_MS)) {
    lastFlush = now;
    sd_log::flush();
  }

  // --- Heartbeat ------------------------------------------------------------
  if (now - lastBeat >= cfg::HEARTBEAT_MS) {
    lastBeat = now;
    Serial.printf("[status] up=%lus | acq=%d | demo=%d | ble=%d | samples=%lu | drops=%lu | research/edu only\n",
                  static_cast<unsigned long>(now / 1000), static_cast<int>(g_acq),
                  static_cast<int>(g_demo), static_cast<int>(ble_stream::connected()),
                  static_cast<unsigned long>(g_sampleIndex),
                  static_cast<unsigned long>(g_ring.dropped()));
  }
}
