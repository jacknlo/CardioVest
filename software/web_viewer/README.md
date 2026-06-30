# CardioVest — Web ECG Viewer

A single-file, dependency-free **Web Bluetooth** viewer for the CardioCore V1 firmware.
It plots the raw multi-channel ECG frames streamed over BLE, and includes a **demo mode**
(synthetic ECG) so you can see it working with no hardware.

> **Research / prototyping / education only.** Not a medical device. It displays **raw**
> samples — no diagnosis, no clinical interpretation. The "mV" axis is an *estimate* from
> `VREF`/`PGA`, for visualization only.

## Run it

### Demo (no hardware, works today)
Just open `index.html` in **Chrome or Edge** (double-click is fine) and click **▶ Demo**.
You'll see a synthetic 8-channel ECG scroll by. Try **Pause**, **Gain**, **Units**, and **Record CSV**.

### Real device (BLE)
Web Bluetooth only works in a **secure context** — `https://` or `http://localhost` (NOT `file://`).
So serve the folder locally:

```bash
cd software/web_viewer
python -m http.server 8000
```

Then open **http://localhost:8000** in Chrome/Edge, flash + power the board, and click
**🔗 Connect (BLE)** → pick **CardioCore-V1**. (Android Chrome works too; iOS Safari does not
support Web Bluetooth — use the demo there.)

## What it does

- **Auto-detects channels** from the frame length: 9 bytes → 2 ch (ADS1292R), 27 bytes → 8 ch (ADS1298).
- Decodes each frame as `24-bit status + N × 24-bit` samples (big-endian, two's complement) —
  matching `firmware/esp32_s3` (`config.h` frame geometry, `ble_stream.h` UUIDs).
- Scrolling per-channel traces with auto-scale + manual **Gain**.
- **Units:** raw counts or estimated **mV** (`counts × VREF / 2^23 / PGA × 1000`).
- **Record CSV:** captures `t_ms, ch1..chN` and downloads `cardiovest_ecg_raw.csv` for offline analysis.

## BLE contract (must match the firmware)

| Item | UUID |
|---|---|
| Service | `c0de0001-feed-4cad-b10c-0000cad10000` |
| Data (notify) | `c0de0002-feed-4cad-b10c-0000cad10000` |

If you change the UUIDs or frame layout in the firmware, update them at the top of `index.html`.

## Status

Works in demo mode now; the BLE path targets the current firmware service. Not yet tested
against real hardware (the ESP32-S3 + AFE are in bring-up). Browser support: Chrome/Edge
(desktop + Android). Firefox/Safari lack Web Bluetooth — demo mode still works there.
