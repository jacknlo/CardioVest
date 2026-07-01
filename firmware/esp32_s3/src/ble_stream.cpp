// ============================================================================
//  CardioVest / CardioCore V1 - BLE raw-sample streaming (NimBLE) impl
//  Notifies RAW ADS1298 frames to a connected research host. No interpretation.
// ============================================================================
#include "ble_stream.h"
#include "config.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

namespace ble_stream {
namespace {

NimBLECharacteristic* g_data      = nullptr;
NimBLECharacteristic* g_mark      = nullptr;
volatile bool         g_connected = false;
volatile uint16_t     g_mtu       = 247;    // negotiated ATT MTU (assume the requested value until told)
volatile bool         g_mtuKnown  = false;  // true once the host negotiates an MTU
bool                  g_mtuWarned = false;

class ServerCb : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* /*server*/) override { g_connected = true; }
  void onDisconnect(NimBLEServer* /*server*/) override {
    g_connected = false;
    g_mtuKnown = false; g_mtu = 247; g_mtuWarned = false;
    NimBLEDevice::startAdvertising();   // allow the host to reconnect
  }
  void onMTUChange(uint16_t mtu, ble_gap_conn_desc* /*desc*/) override {
    g_mtu = mtu; g_mtuKnown = true;
  }
};
ServerCb g_serverCb;

}  // namespace

void begin(const char* deviceName) {
  NimBLEDevice::init(deviceName);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  // Request a large ATT MTU so batched notifications fit (up to 6 * 31 = 186 B).
  // Default (23) → 20-byte payload would truncate even one 31-byte frame.
  // The host still negotiates the final value; desktop Chrome/Edge grant ~247+.
  NimBLEDevice::setMTU(247);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(&g_serverCb);

  NimBLEService* svc = server->createService(SVC_UUID);
  g_data = svc->createCharacteristic(DATA_UUID, NIMBLE_PROPERTY::NOTIFY);
  g_mark = svc->createCharacteristic(MARK_UUID, NIMBLE_PROPERTY::NOTIFY);  // event markers
  svc->createCharacteristic(CTRL_UUID, NIMBLE_PROPERTY::WRITE);  // future start/stop/config
  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->setScanResponse(true);
  NimBLEDevice::startAdvertising();
}

bool connected() { return g_connected; }

void sendFrame(const uint8_t* frame, size_t len) {
  if (!g_connected || g_data == nullptr) return;
  // A notification payload must fit the negotiated MTU (ATT_MTU - 3). Until the
  // host negotiates the MTU we MUST assume the 23-byte default (20-byte payload):
  // sending a 186-byte batch before onMTUChange fires gets silently truncated by
  // the stack, corrupting frames on the host. So the safe payload is (MTU-3) once
  // known, else 20. Skip anything larger rather than emit garbage.
  const size_t maxPayload = g_mtuKnown ? (g_mtu > 3 ? (size_t)(g_mtu - 3) : 0) : 20;
  if (len > maxPayload) {
    // Only warn about a genuinely-too-small negotiated MTU; the brief pre-
    // negotiation window is expected and self-heals once onMTUChange fires.
    if (g_mtuKnown && !g_mtuWarned) {
      g_mtuWarned = true;
      Serial.printf("[ble] negotiated MTU %u too small for %u-byte batch; notifications skipped. Lower cfg::BATCH_FRAMES.\n",
                    (unsigned)g_mtu, (unsigned)len);
    }
    return;
  }
  g_data->setValue(frame, len);
  g_data->notify();   // NimBLE 1.4.x notify() returns void; drops (host buffers full) are not surfaced by this API
}

void sendMarker(uint32_t markerId, uint32_t sampleIndex) {
  if (!g_connected || g_mark == nullptr) return;
  uint8_t p[8];
  p[0]= markerId           & 0xFF; p[1]=(markerId    >> 8)  & 0xFF;
  p[2]=(markerId    >> 16) & 0xFF; p[3]=(markerId    >> 24) & 0xFF;
  p[4]= sampleIndex        & 0xFF; p[5]=(sampleIndex >> 8)  & 0xFF;
  p[6]=(sampleIndex >> 16) & 0xFF; p[7]=(sampleIndex >> 24) & 0xFF;
  g_mark->setValue(p, 8);
  g_mark->notify();
}

}  // namespace ble_stream
