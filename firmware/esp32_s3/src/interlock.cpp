// ============================================================================
//  CardioVest / CardioCore V1 - safety interlock impl
//  Encodes "battery-only during body-connected measurement". The supporting
//  hardware (USB_PRESENT / AFE_ENABLE) is TBD (B9/B10); until wired, the
//  interlock fails SAFE and inhibits real-AFE acquisition, unless the bench
//  opt-out cfg::ALLOW_AFE_WITHOUT_USB_DETECT is set (no-electrode testing).
//  Research convenience only - NOT a certified medical safety system.
// ============================================================================
#include "interlock.h"
#include "config.h"
#include "pins.h"
#include <Arduino.h>

namespace interlock {
namespace {

bool g_afeEnabled = false;
bool g_warned     = false;

}  // namespace

void begin() {
  if (pins::AFE_ENABLE >= 0) {
    pinMode(pins::AFE_ENABLE, OUTPUT);
    digitalWrite(pins::AFE_ENABLE, LOW);   // AFE path disabled until enabled
  }
  if (pins::USB_PRESENT >= 0) {
    pinMode(pins::USB_PRESENT, INPUT);
  }
  g_afeEnabled = false;
}

bool usbPresent() {
  if (pins::USB_PRESENT < 0) return false;       // unknown (see acquisitionPermitted)
  return digitalRead(pins::USB_PRESENT) == HIGH;
}

void setAfeEnabled(bool en) {
  g_afeEnabled = en;
  if (pins::AFE_ENABLE >= 0) digitalWrite(pins::AFE_ENABLE, en ? HIGH : LOW);
}

bool afeEnabled() { return g_afeEnabled; }

bool acquisitionPermitted() {
  if (!cfg::ENABLE_INTERLOCK) return true;

  if (pins::USB_PRESENT < 0) {
    // Detection hardware not wired yet (B9/B10): cannot verify battery-only.
    if (!g_warned) {
      g_warned = true;
      if (cfg::ALLOW_AFE_WITHOUT_USB_DETECT) {
        Serial.println(F("[interlock] WARNING: USB-present detection not wired (B9/B10)."));
        Serial.println(F("[interlock] Bench opt-out ENABLED: AFE permitted with NO hardware"));
        Serial.println(F("[interlock] battery-only enforcement. Use ONLY with no electrodes on"));
        Serial.println(F("[interlock] a person (see docs/Safety_Research_Use.md)."));
      } else {
        Serial.println(F("[interlock] USB-present detection not wired (B9/B10):"));
        Serial.println(F("[interlock] failing SAFE - real-AFE acquisition INHIBITED."));
        Serial.println(F("[interlock] Wire the detector, or for bench/no-electrode testing set"));
        Serial.println(F("[interlock] cfg::ALLOW_AFE_WITHOUT_USB_DETECT (see Safety_Research_Use.md)."));
      }
    }
    return cfg::ALLOW_AFE_WITHOUT_USB_DETECT;
  }

  // Hardware present: inhibit acquisition while on non-isolated USB.
  return !usbPresent();
}

}  // namespace interlock
