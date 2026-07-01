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

bool     g_mounted = false;
File     g_file;
uint32_t g_seq = 0;

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
  for (uint32_t i = 0; i < 10000; ++i) {
    snprintf(name, sizeof(name), "/ecg_%04lu.bin", static_cast<unsigned long>(i));
    if (!SD.exists(name)) { g_seq = i; break; }
  }
  g_file = SD.open(name, FILE_WRITE);
  return static_cast<bool>(g_file);
}

void writeFrame(const uint8_t* frame, size_t len) {
  if (!g_file) return;
  g_file.write(frame, len);
}

void flush() {
  if (g_file) g_file.flush();
}

void stopSession() {
  if (g_file) g_file.close();
}

bool active() { return static_cast<bool>(g_file); }

}  // namespace sd_log
