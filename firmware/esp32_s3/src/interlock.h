// ============================================================================
//  CardioVest / CardioCore V1 - safety interlock
// ----------------------------------------------------------------------------
//  Encodes the project's core electrical-safety rule:
//
//    DO NOT acquire body-connected ECG while the board is powered from
//    non-isolated USB. Body-connected measurement must be BATTERY-ONLY.
//
//  The supporting hardware (USB_PRESENT sensing, AFE_ENABLE control) is not yet
//  assigned in the schematic (blockers B9 / B10). Until then this module
//  degrades gracefully and warns that the interlock is not enforced in hardware.
//  This is a research-safety convenience - NOT a certified medical safety
//  system.
// ============================================================================
#pragma once

namespace interlock {

void begin();
bool usbPresent();          // true if powered/charging from USB (best-effort)
void setAfeEnabled(bool en);
bool afeEnabled();

// Safety gate: returns false when acquisition should be inhibited (e.g. USB
// present while the interlock is enabled). When detection hardware is missing,
// returns true but warns - the operator must ensure battery-only operation.
bool acquisitionPermitted();

}  // namespace interlock
