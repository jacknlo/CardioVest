# microSD logging bring-up (bench)

> How to add a **microSD breakout** to the ESP32-S3 dev board (which has no SD slot)
> to test on-board logging — **no soldering, just jumper wires** on a breadboard.
> **Research / education only.** GPIO numbers match [`../include/pins.h`](../include/pins.h).

The firmware already logs raw transport frames to the card (`src/sd_log.cpp`, SPI mode).
Without a card it prints `microSD not available` and keeps running — expected.

## What to buy
A cheap "microSD / TF card module" (~US$1–3). Two common kinds:
- **3.3 V module** (bare, no regulator) → power **VCC = 3.3 V**. Simplest for the ESP32.
- **Blue module with AMS1117 + level shifter** → power **VCC = 5 V** (the board's 5V/USB pin);
  the SPI lines stay at 3.3 V (the module level-shifts).

> ⚠️ Never drive 5 V into an ESP32-S3 GPIO. Power may be 3.3 V or 5 V (module dependent);
> **SPI/control lines are always 3.3 V.**

## Wiring (jumpers)

| SD module | ESP32-S3 GPIO | `pins.h` |
|---|---|---|
| CS / SS | IO9 | `SD_CS` |
| SCK / CLK | IO12 | `ADS_SCLK` (shared SPI) |
| MOSI / DI | IO11 | `ADS_MOSI` (shared SPI) |
| MISO / DO | IO13 | `ADS_MISO` (shared SPI) |
| VCC | 3V3 (bare module) **or** 5V (level-shifted module) | — |
| GND | GND | — |

The SD shares the SPI bus with the ADS129x (SCK/MOSI/MISO common; only CS differs), so it
can sit on the same breadboard as the ADS1292R.

## Card prep
- Format the card as **FAT32** (cards ≤ 32 GB are easiest; larger ones may need FAT32 tooling).
- Insert before power-up.

## Test
1. Wire + insert the card, then reset/boot the board (default `cardiocore_v1` env is fine —
   the demo stream also logs when a card is present).
2. Serial should show `microSD session open.` (instead of `microSD not available`).
3. A log file `/ecg_0000.bin`, `/ecg_0001.bin`, … is created. Each record is one **transport
   frame**: `[u32 sample_index LE][status(3)][N × 24-bit channels]` — 31 B (ADS1298) / 13 B (ADS1292R).

### Parsing a log offline (Python sketch)
```python
import struct
REC = 31          # 4 + 3 + 8*3 for ADS1298 (use 13 for ADS1292R)
NCH = (REC - 4 - 3)//3
data = open("ecg_0000.bin","rb").read()
for off in range(0, len(data)-REC+1, REC):
    seq = struct.unpack_from("<I", data, off)[0]
    ch = []
    for c in range(NCH):
        b = data[off+4+3+c*3 : off+4+3+c*3+3]      # 24-bit big-endian, signed
        v = int.from_bytes(b, "big", signed=True)
        ch.append(v)
    # seq, ch  -> your analysis (raw counts)
```

## Notes
- Bench-only: the **CardioCore V1** board has a microSD socket built in (**J2**), so the module
  is just for testing on the dev board.
- SD and ADS share the SPI bus; today they run sequentially in `loop()`, so there's no contention.
  When both are wired, confirm the SD library restores SPI settings between accesses.
