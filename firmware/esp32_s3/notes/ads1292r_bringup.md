# ADS1292R module bring-up (Phase 1)

> Wiring + steps to bring up the firmware on a **2-channel ADS1292R dev module**
> (the eBay module purchased for Phase 1, ships with a cable + 3 electrodes)
> connected to the **ESP32-S3-WROOM-1-N16R8** dev board. **Research / education only.**

The firmware already supports this via the `cardiocore_v1_ads1292r` build (2 channels,
9-byte frames, ADS1292R register map). GPIO numbers below match [`../include/pins.h`](../include/pins.h).

## Wiring (PRELIMINARY — verify against the module silkscreen on arrival)

The module's header is silkscreened: `5V GND MISO MOSI CLK PWDN` / `5V GND DRDY SCK CSO START`, plus a `3.3V` pad.

| Module pin | ESP32-S3 GPIO | `pins.h` | Note |
|---|---|---|---|
| **3.3V** | 3V3 | — | Power the module at **3.3 V** (ESP32 is 3.3 V logic) |
| **GND** | GND | — | Common ground |
| **SCK** | IO12 | `ADS_SCLK` | SPI clock |
| **MOSI** | IO11 | `ADS_MOSI` | → ADS DIN |
| **MISO** | IO13 | `ADS_MISO` | → ADS DOUT |
| **CSO** (CS) | IO10 | `ADS_CS` | chip select (active low) |
| **DRDY** | IO14 | `ADS_DRDY` | data-ready IRQ |
| **START** | IO15 | `ADS_START` | conversion start |
| **PWDN** | IO17 | `ADS_PWDN` | power-down (active low) |
| **RESET** | IO16 | `ADS_RESET` | *if the module exposes it — see open questions* |

> ⚠️ **Do NOT feed 5 V into any ESP32-S3 GPIO.** ADS1292R digital supply is 1.7–3.6 V,
> so 3.3 V logic is correct. Powering the module's 3.3V pin from the ESP32 3V3 is simplest;
> if you use the 5V pin instead, the module's on-board regulator makes 3.3 V, but keep all
> SPI/control lines at 3.3 V.

## Open questions (confirm with the module in hand)
- **RESET pin:** the visible header shows `PWDN` but not `RESET`. If RESET isn't broken out,
  leave `ADS_RESET` (IO16) unconnected — the firmware also issues the SPI `RESET` command
  (0x06), and the pin pulse is harmless if NC. Confirm.
- **CLK pin:** the ADS1292R device master clock. If the module uses its internal oscillator
  (CLKSEL strapped on-board), leave `CLK` unconnected. Confirm.
- **CSO label:** confirm `CSO` is the active-low chip select wired to `ADS_CS`.

## Flash & run
```bash
cd firmware/esp32_s3
pio run -e cardiocore_v1_ads1292r -t upload
pio device monitor       # or capture serial
```
The web viewer auto-detects 2 channels (9-byte frames).

## First test — NO body connection
1. Wire + power per the table, flash the `ads1292r` build.
2. Serial should show `ADS1292R detected, ID=0x..` (expected ~`0x73`). If "NOT detected",
   recheck wiring / power / CS.
3. Enable the ADS1292R **internal test signal** (set `CONFIG2 = 0xA3` in `configureDefault()`)
   to see a square wave — this validates the whole SPI + acquisition chain with **no electrodes**.
4. Only after the bench test passes: battery-only electrode experiments (**never on USB**,
   per `docs/Safety_Research_Use.md`).
