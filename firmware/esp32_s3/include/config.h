// ============================================================================
//  CardioVest / CardioCore V1 - firmware build configuration
//  Research / education only. NOT a medical device. Raw data only.
// ============================================================================
#pragma once
#include <stdint.h>

#define FW_PROJECT "CardioVest / CardioCore V1"
#define FW_VERSION "v0.2.0-dev"

namespace cfg {

// --- ADS1298 acquisition frame geometry -------------------------------------
// One RDATAC frame = 24-bit status word + 8 channels x 24-bit samples = 27 bytes.
static constexpr uint8_t  NUM_CHANNELS = 8;
static constexpr uint8_t  CH_BYTES     = 3;                       // 24-bit/channel
static constexpr uint8_t  STATUS_BYTES = 3;                       // 24-bit status
static constexpr uint16_t FRAME_BYTES  = STATUS_BYTES + NUM_CHANNELS * CH_BYTES; // 27

// --- Default sample rate (candidate; finalize against datasheet) -------------
// ADS1298 CONFIG1 DR[2:0] selects the rate; 500 SPS is a reasonable ECG start.
static constexpr uint16_t DEFAULT_SPS = 500;  // TBD - verify

// --- Buffering ---------------------------------------------------------------
static constexpr uint16_t RING_FRAMES = 256;  // acquisition ring-buffer depth

// --- Feature flags -----------------------------------------------------------
static constexpr bool ENABLE_BLE       = true;
static constexpr bool ENABLE_SD        = true;
static constexpr bool ENABLE_INTERLOCK = true;   // battery-only body-measurement gate

// --- Misc --------------------------------------------------------------------
static constexpr uint32_t SERIAL_BAUD  = 115200;
static constexpr uint32_t HEARTBEAT_MS = 5000;
static constexpr uint32_t SD_FLUSH_MS  = 1000;   // periodic log flush

}  // namespace cfg
