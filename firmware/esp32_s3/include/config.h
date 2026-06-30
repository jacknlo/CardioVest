// ============================================================================
//  CardioVest / CardioCore V1 - firmware build configuration
//  Research / education only. NOT a medical device. Raw data only.
// ============================================================================
#pragma once
#include <stdint.h>

#define FW_PROJECT "CardioVest / CardioCore V1"
#define FW_VERSION "v0.2.0-dev"

// --- AFE selection (ADS129x family) -----------------------------------------
// ADS1298  = the CardioCore V1 board (8 channels).
// ADS1292R = a 2-channel dev module (same SPI command family) for early
//            bring-up on a breadboard before the CardioCore V1 PCB exists.
// Override from the build: platformio.ini env `cardiocore_v1_ads1292r`
// (adds -D AFE_TYPE=AFE_ADS1292R).
#define AFE_ADS1298  1
#define AFE_ADS1292R 2
#ifndef AFE_TYPE
#define AFE_TYPE AFE_ADS1298
#endif

namespace cfg {

#if AFE_TYPE == AFE_ADS1292R
static constexpr const char* AFE_NAME     = "ADS1292R";
static constexpr uint8_t     NUM_CHANNELS = 2;   // 2 ECG channels
#else
static constexpr const char* AFE_NAME     = "ADS1298";
static constexpr uint8_t     NUM_CHANNELS = 8;   // 8 ECG channels
#endif

// --- Acquisition frame geometry ---------------------------------------------
// One RDATAC frame = 24-bit status word + NUM_CHANNELS x 24-bit samples.
//   ADS1298  -> 3 + 8*3 = 27 bytes
//   ADS1292R -> 3 + 2*3 =  9 bytes
static constexpr uint8_t  CH_BYTES     = 3;                       // 24-bit/channel
static constexpr uint8_t  STATUS_BYTES = 3;                       // 24-bit status
static constexpr uint16_t FRAME_BYTES  = STATUS_BYTES + NUM_CHANNELS * CH_BYTES;

// --- Default sample rate (candidate; finalize against datasheet) -------------
// ADS1298 CONFIG1 DR[2:0] selects the rate; 500 SPS is a reasonable ECG start.
static constexpr uint16_t DEFAULT_SPS = 500;  // TBD - verify

// --- Buffering ---------------------------------------------------------------
static constexpr uint16_t RING_FRAMES = 256;  // acquisition ring-buffer depth

// --- Feature flags -----------------------------------------------------------
static constexpr bool ENABLE_BLE       = true;
static constexpr bool ENABLE_SD        = true;
static constexpr bool ENABLE_INTERLOCK = true;   // battery-only body-measurement gate

// When no AFE is detected, synthesize ECG frames so the BLE/SD pipeline can be
// tested end-to-end with just the ESP32-S3 (no analog front-end). Dev aid only;
// synthetic data is clearly not a real measurement.
static constexpr bool ENABLE_DEMO_STREAM = true;

// --- Misc --------------------------------------------------------------------
static constexpr uint32_t SERIAL_BAUD  = 115200;
static constexpr uint32_t HEARTBEAT_MS = 5000;
static constexpr uint32_t SD_FLUSH_MS  = 1000;   // periodic log flush

}  // namespace cfg
