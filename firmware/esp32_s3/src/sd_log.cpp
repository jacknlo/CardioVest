// ============================================================================
//  CardioVest / CardioCore V1 - microSD raw-frame logger impl
//  Appends RAW frames to a binary file over the shared SPI bus.
//  NOTE: the microSD shares the SPI bus with the ADS1298; bus arbitration /
//  transaction discipline must be confirmed during bring-up.
// ============================================================================
#include "sd_log.h"
#include "config.h"
#include "pins.h"
#include <SPI.h>
#include <SD.h>

namespace sd_log {
namespace {

bool     g_mounted   = false;
File     g_file;
uint32_t g_seq       = 0;
uint32_t g_lostBytes = 0;      // bytes that failed to write (card full/removed/error)
uint32_t g_lastRemountMs = 0;  // backoff timer for remount attempts

}  // namespace

bool begin() {
  // Reuse the SPI bus already initialized by the ADS1298 driver.
  g_mounted = SD.begin(pins::SD_CS, SPI);
  return g_mounted;
}

bool startSession() {
  if (!g_mounted) return false;
  char name[24];
  // Pick the first unused /ecg_NNNN.bin so a reboot never appends onto a prior
  // session (FILE_WRITE opens with O_APPEND, and g_seq resets to 0 each boot).
  bool found = false;
  for (uint32_t i = 0; i < 10000; ++i) {
    snprintf(name, sizeof(name), "/ecg_%04lu.bin", static_cast<unsigned long>(i));
    if (!SD.exists(name)) { g_seq = i; found = true; break; }
  }
  if (!found) return false;   // all 10000 names taken: refuse rather than append onto ecg_9999.bin
  g_file = SD.open(name, FILE_WRITE);
  return static_cast<bool>(g_file);
}

void writeFrame(const uint8_t* frame, size_t len) {
  if (!g_file) {
    // Session is down (card removed, full, or write error). Try to recover, but
    // rate-limit so we don't hammer SD.begin() on every frame. Data during the
    // outage is genuinely lost - count it so the loss is visible, not silent.
    g_lostBytes += len;
    const uint32_t now = millis();
    if (now - g_lastRemountMs >= 2000) {
      g_lastRemountMs = now;
      g_mounted = SD.begin(pins::SD_CS, SPI);
      if (g_mounted) startSession();
    }
    return;
  }
  if (g_file.write(frame, len) != len) {
    // Short write: card full or gone. Close and drop to the recovery path above.
    g_lostBytes += len;
    g_file.close();
  }
}

uint32_t bytesLost() { return g_lostBytes; }

void flush() {
  if (g_file) g_file.flush();
}

void stopSession() {
  if (g_file) g_file.close();
}

bool active() { return static_cast<bool>(g_file); }

}  // namespace sd_log
