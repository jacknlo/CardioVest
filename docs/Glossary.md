# CardioVest Glossary

This glossary defines technical terms used across the CardioVest research platform
and the CardioCore V1 hardware module. Definitions are written in an engineering /
research sense. CardioVest is a platform for **research, prototyping, and education
only**; it is **not** a certified medical device, and none of the definitions below
imply diagnostic, clinical, or patient-monitoring capability.

---

### ADC (Analog-to-Digital Converter)
A circuit that converts a continuous analog voltage into discrete numerical samples.
In CardioCore V1 the ADC stage is integrated inside the ADS1298 and provides 24-bit
resolution per channel.

### ADS1298
A Texas Instruments 8-channel, 24-bit, simultaneous-sampling analog front-end (AFE)
designed for biopotential acquisition. It integrates per-channel programmable-gain
amplifiers, sigma-delta ADCs, an SPI interface, and right-leg-drive support, and is
the core signal-acquisition IC of CardioCore V1.

### AFE (Analog Front-End)
The analog signal-conditioning subsystem that sits between the electrodes and the
digital domain, typically handling buffering, amplification, filtering, and
analog-to-digital conversion. On CardioCore V1 the AFE is the ADS1298.

### BLE (Bluetooth Low Energy)
A low-power wireless protocol used by CardioCore V1 to stream acquired sample data
to a host device. Data is exposed through BLE GATT services and characteristics.

### BOM (Bill of Materials)
A structured list of every component required to build the board, including
reference designators, part numbers, values, footprints, and quantities. The BOM is
used for procurement and assembly of CardioCore V1.

### Common-mode rejection (CMRR — Common-Mode Rejection Ratio)
A measure of how well a differential amplifier suppresses signals common to both of
its inputs (such as mains-coupled interference) relative to the differential signal
of interest. Higher CMRR generally yields cleaner biopotential recordings.

### Decoupling
The practice of placing capacitors close to a device's power pins to supply transient
current locally and suppress supply-rail noise. Good decoupling is important around
the ADS1298, REF5025, and ESP32-S3 to preserve signal integrity.

### DRDY (Data Ready)
An output signal from the ADS1298 that indicates a new set of conversion results is
available to be read over SPI. The MCU typically treats DRDY as an interrupt to
trigger a sample read.

### ECG (Electrocardiography)
The measurement of the heart's electrical activity as a function of time, recorded as
voltage differences between electrodes placed on the body. In this project ECG refers
strictly to signal acquisition for research and education; CardioVest performs no
diagnosis or clinical interpretation of these signals.

### Electrode
A conductive contact applied to the skin that transduces ionic body currents into an
electronic signal for the AFE. CardioCore V1 supports 10 electrode inputs: RA, LA, LL,
RL/RLD, V1, V2, V3, V4, V5, and V6.

### ESP32-S3
A dual-core microcontroller SoC from Espressif with integrated Wi-Fi and BLE. The
CardioCore V1 design uses the ESP32-S3-WROOM-1-N16R8 module (16 MB flash, 8 MB PSRAM)
to manage the AFE, BLE streaming, and microSD logging.

### GATT (Generic Attribute Profile)
The BLE framework that organizes data into services and characteristics, defining how
a client reads, writes, and subscribes to values. CardioCore V1 publishes its
streaming data through a GATT structure.

### Lead
A signal derived from the voltage difference between electrodes (or a defined
combination of electrodes), as opposed to a single electrode contact. The distinction
matters: an **electrode** is a physical sensor on the body, while a **lead** is a
computed/differential vector between electrode potentials.

### LiPo (Lithium Polymer battery)
A rechargeable battery chemistry used to power the board. CardioCore V1 uses a single
1S LiPo cell, charged over USB-C.

### microSD
A removable flash memory card used by CardioCore V1 for local logging of acquired
sample data, accessed over an SPI or SDIO interface.

### PCB (Printed Circuit Board)
The fabricated board that mechanically supports and electrically interconnects the
project's components via etched copper traces. CardioCore V1 is implemented as a
multi-layer PCB.

### PGA (Programmable-Gain Amplifier)
An amplifier whose gain can be configured in software, allowing the input signal to be
scaled to use the ADC's dynamic range effectively. The ADS1298 includes a PGA on each
channel.

### REF5025
A Texas Instruments precision voltage-reference IC providing a stable 2.5 V output. In
CardioCore V1 it serves as the external precision reference for the ADS1298 to improve
measurement accuracy and stability.

### RLD / DRL (Right-Leg Drive / Driven Right Leg)
An active feedback technique that drives a body-bias electrode (commonly at the right
leg, RL) to reduce common-mode interference and improve recording quality. The ADS1298
provides an integrated right-leg-drive amplifier for this purpose.

### Sampling rate
The number of samples acquired per channel per second, expressed in samples per second
(SPS) or hertz. The target sampling rate for CardioCore V1 is **TBD** and will be set
to balance signal fidelity against power, bandwidth, and storage constraints.

### SPI (Serial Peripheral Interface)
A synchronous serial bus using clock, data-in, data-out, and chip-select lines. It is
the primary digital interface between the ESP32-S3 and the ADS1298 (and is also used
for microSD access).

### TBD (To Be Determined)
A marker used throughout the project documentation to flag an engineering value or
decision that has not yet been finalized. A TBD is an explicit placeholder, never a
fabricated final specification.

### USB-C
A reversible connector standard used by CardioCore V1 for battery charging and for
programming/communication with the ESP32-S3.

### V1–V6 (Precordial electrode positions)
The named precordial (chest) electrode positions used in standard 10-electrode ECG
electrode placement, referenced here by name only (V1, V2, V3, V4, V5, V6) as inputs
to the CardioCore V1 acquisition front-end.
