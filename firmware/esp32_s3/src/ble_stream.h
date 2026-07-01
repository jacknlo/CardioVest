// ============================================================================
//  CardioVest / CardioCore V1 - BLE raw-sample streaming (NimBLE)
// ----------------------------------------------------------------------------
//  Exposes a custom GATT service that notifies RAW ADS1298 frames to a
//  connected research host. Transports raw bytes only - no interpretation.
// ============================================================================
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace ble_stream {

// Custom 128-bit UUIDs (project-specific, not a standard SIG service).
//   Service : data + control for raw ECG-research streaming
//   DATA    : NOTIFY characteristic carrying raw frames
//   CTRL    : WRITE characteristic for simple start/stop/config (optional)
constexpr char SVC_UUID[]  = "c0de0001-feed-4cad-b10c-0000cad10000";
constexpr char DATA_UUID[] = "c0de0002-feed-4cad-b10c-0000cad10000";
constexpr char CTRL_UUID[] = "c0de0003-feed-4cad-b10c-0000cad10000";
constexpr char MARK_UUID[] = "c0de0004-feed-4cad-b10c-0000cad10000";  // event markers (notify)

void begin(const char* deviceName);          // init NimBLE, advertise
bool connected();                            // host connected?
void sendFrame(const uint8_t* frame, size_t len);   // notify raw transport frame
void sendMarker(uint32_t markerId, uint32_t sampleIndex); // notify [u32 markerId][u32 sample_index] LE (resent a few times for reliability; viewer de-dups by id)

}  // namespace ble_stream
