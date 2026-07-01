// ============================================================================
//  CardioVest / CardioCore V1 - microSD raw-frame logger
// ----------------------------------------------------------------------------
//  Appends RAW ADS1298 frames to a binary log file on the microSD card
//  (over the shared SPI bus). Stores raw bytes only - no interpretation.
// ============================================================================
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace sd_log {

bool begin();                                    // mount the card
bool startSession();                             // open a fresh log file
void writeFrame(const uint8_t* frame, size_t len);
void flush();                                    // commit buffered writes
void stopSession();                              // close the current file
bool active();                                   // a session file is open?
uint32_t bytesLost();                            // bytes dropped during card outage/error

}  // namespace sd_log
