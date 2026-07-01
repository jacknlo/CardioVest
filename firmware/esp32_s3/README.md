# CardioCore V1 — ESP32-S3 Firmware

Firmware for the **CardioCore V1** board of the **CardioVest** project — an 8-channel, battery-powered ECG acquisition board (ESP32-S3 + TI ADS1298).

> **Research / prototyping / education only.** This firmware is **NOT** a medical device and is not for diagnosis, treatment, patient monitoring, emergency use, or any clinical decision-making. It only acquires, buffers, streams, and logs **raw** samples — it performs **no** clinical interpretation.

---

## 1. What it does

```
 Electrodes ─▶ ADS1298 AFE ──(SPI + DRDY IRQ)──▶ ESP32-S3
                                                   │
                                                   ├─▶ ring buffer (RAW 27-byte frames)
                                                   │        ├─▶ BLE GATT notify  ─▶ host
                                                   │        └─▶ microSD log (.bin)
                                                   └─▶ safety interlock gate
```

Each BLE/SD **transport frame** = a little-endian **u32 sample counter** prepended to one raw ADS record (24-bit status + N × 24-bit channels): **31 bytes** (ADS1298) or **13 bytes** (ADS1292R). The counter enables dropped-frame detection and annotation time-sync.

Press the **BOOT button** (SW2 on CardioCore V1) to drop a **Zio-style event marker** — timestamped with the current sample index and emitted on serial + a BLE marker characteristic (`c0de0004…`).

> **Demo stream:** if **no AFE is detected**, the firmware synthesizes an ECG-like signal and streams it over BLE/SD anyway (`ENABLE_DEMO_STREAM` in `config.h`). This lets you validate the whole wireless pipeline (ESP32-S3 → BLE → web viewer) with **just the dev board**, before any ADS129x is wired. Synthetic data is obviously not a real measurement.

## 2. Module layout

```
firmware/esp32_s3/
├── platformio.ini        # build config (env: cardiocore_v1)
├── include/
│   ├── config.h          # frame geometry, sample rate, feature flags
│   └── pins.h            # ESP32-S3 pin map (from the verified EDIF netlist)
├── src/
│   ├── main.cpp          # setup/loop orchestration + DRDY ISR
│   ├── ads1298.{h,cpp}   # ADS1298 SPI driver (register I/O + raw frame read)
│   ├── ring_buffer.h     # SPSC fixed-size frame FIFO
│   ├── ble_stream.{h,cpp}# NimBLE raw-frame notify service
│   ├── sd_log.{h,cpp}    # microSD binary frame logger
│   └── interlock.{h,cpp} # battery-only-measurement safety gate (B9/B10)
└── notes/firmware_plan.md
```

## 3. Pin map (ESP32-S3 / U1 — from `schematics/CardioCore_V1.edif`)

| Function | Net | GPIO | | Function | Net | GPIO |
|---|---|---|---|---|---|---|
| ADS SCLK | SPI_SCLK | IO12 | | ADS CS | ADS_CS | IO10 |
| ADS MOSI/DIN | SPI_MOSI | IO11 | | ADS DRDY | ADS_DRDY | IO14 |
| ADS MISO/DOUT | SPI_MISO | IO13 | | ADS START | ADS_START | IO15 |
| ADS RESET | ADS_RESET | IO16 | | ADS PWDN | ADS_PWDN | IO17 |
| microSD CS | SD_CS | IO9 | | CHG INT | CHG_INT | IO6 |
| I²C SDA | I2C_SDA | IO4 | | I²C SCL | I2C_SCL | IO5 |
| EXP GPIO1 | EXP_GPIO1 | IO7 | | EXP GPIO2 | EXP_GPIO2 | IO8 |
| USB D− | USB_DN | IO19 | | USB D+ | USB_DP | IO20 |

`USB_PRESENT` and `AFE_ENABLE` for the interlock are **not yet assigned** in hardware (blockers **B9/B10**) — see [`pins.h`](include/pins.h).

## 4. Build & flash (PlatformIO)

```bash
cd firmware/esp32_s3
pio run                    # build
pio run -t upload          # flash over USB-C
pio device monitor         # serial @ 115200
```

Board is set to `esp32-s3-devkitc-1` (same MCU family) until a custom CardioCore V1 board JSON is added. BLE uses **NimBLE-Arduino** (pinned in `platformio.ini`).

> This board has **two USB-C ports**. The default config routes the serial console to the **UART bridge** port (one cable does flash + monitor). To use the native-USB port instead, set `ARDUINO_USB_CDC_ON_BOOT=1`.

### Choosing the AFE (ADS1298 vs ADS1292R)

The firmware supports the whole ADS129x family via a compile-time switch (`AFE_TYPE` in `config.h`), which sets the channel count and frame size automatically:

```bash
pio run                              # default: ADS1298 (8 ch, 27-byte frames) - CardioCore V1
pio run -e cardiocore_v1_ads1292r    # 2-ch ADS1292R dev module (9-byte frames) - early bring-up
```

Use the `ads1292r` env with a 2-channel ADS1292R breakout to validate the firmware (SPI, ID read, internal test signal) on a breadboard before the CardioCore V1 PCB exists. The driver, ring buffer, BLE, and SD paths are shared; only the register map and frame size differ.

## 5. Status

**Compiles cleanly; not yet hardware-verified.** Builds for the ESP32-S3 under
PlatformIO (`pio run` → SUCCESS, ~93 s; RAM ~11 %, Flash ~9 %; Arduino core +
NimBLE + SD + SPI all link, no warnings from project sources). The structure,
ADS1298 register/command set, SPI transactions, ring buffer, BLE service, SD
logger, and interlock are implemented, but:

- Several ADS1298 register values in `configureDefault()` are **candidates** and must be verified against the datasheet (TI SBAS459) and the AFE review — see [`../../docs/AFE_Verification_Report.md`](../../docs/AFE_Verification_Report.md).
- ⚠️ **Finding F1 (clock):** the schematic straps `CLKSEL` to GND (external clock) with no clock source; until that is fixed in flux.ai, the ADS1298 may not run on real hardware. See [`../../docs/Flux_Change_List.md`](../../docs/Flux_Change_List.md).
- The microSD shares the SPI bus with the ADS1298; bus arbitration must be confirmed during bring-up.

## 6. Safety

- Research / prototyping / education only — **not** a medical device; no diagnosis, treatment, monitoring, or emergency use; no clinical interpretation.
- The `interlock` module encodes **battery-only operation during body-connected measurement**. The detection/enable hardware is still TBD (B9/B10), so the firmware **warns** that the interlock is not yet enforced in hardware — the operator must ensure battery-only operation. See [`../../docs/Safety_Research_Use.md`](../../docs/Safety_Research_Use.md).

## 7. Related

- [`notes/firmware_plan.md`](notes/firmware_plan.md) · [`../../docs/Bringup_Plan.md`](../../docs/Bringup_Plan.md) · [`../../docs/AFE_Verification_Report.md`](../../docs/AFE_Verification_Report.md)
