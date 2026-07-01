// ============================================================================
//  CardioVest / CardioCore V1 - safety interlock
// ----------------------------------------------------------------------------
//  Encodes the project's core electrical-safety rule:
//
//    DO NOT acquire body-connected ECG while the board is powered from
//    non-isolated USB. Body-connected measurement must be BATTERY-ONLY.
//
//  The supporting hardware (USB_PRESENT sensing, AFE_ENABLE control) is not yet
//  assigned in the schematic (blockers B9 / B10). Until then this module fails
//  SAFE: it inhibits real-AFE acquisition (the demo stream is unaffected) unless
//  the bench opt-out cfg::ALLOW_AFE_WITHOUT_USB_DETECT is set for no-electrode
//  testing. This is a research-safety convenience - NOT a certified medical
//  safety system.
// ============================================================================
#pragma once

namespace interlock {

void begin();
bool usbPresent();          // true if powered/charging from USB (best-effort)
void setAfeEnabled(bool en);
bool afeEnabled();

// Safety gate: returns false when acquisition should be inhibited. USB present
// (detector wired) -> false. Detector NOT wired (B9/B10) -> fails SAFE: returns
// false (inhibits real-AFE acquisition) and warns, unless the bench opt-out
// cfg::ALLOW_AFE_WITHOUT_USB_DETECT is set. The synthetic demo stream is
// unaffected (it never consults the interlock).
bool acquisitionPermitted();

}  // namespace interlock
