// ============================================================================
//  CardioVest / CardioCore V1 - BLE raw-sample streaming (NimBLE) impl
//  Notifies RAW ADS1298 frames to a connected research host. No interpretation.
// ============================================================================
#include "ble_stream.h"
#include "config.h"
#include <NimBLEDevice.h>

namespace ble_stream {
namespace {

NimBLECharacteristic* g_data      = nullptr;
NimBLECharacteristic* g_mark      = nullptr;
volatile bool         g_connected = false;

class ServerCb : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* /*server*/) override { g_connected = true; }
  void onDisconnect(NimBLEServer* /*server*/) override {
    g_connected = false;
    NimBLEDevice::startAdvertising();   // allow the host to reconnect
  }
};
ServerCb g_serverCb;

}  // namespace

void begin(const char* deviceName) {
  NimBLEDevice::init(deviceName);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  // Request a larger ATT MTU. The default (23) yields a 20-byte notify payload,
  // which would TRUNCATE the 27-byte ADS1298 frame. 64 covers 27 + overhead
  // (and the 9-byte ADS1292R frame). The host still negotiates the final value.
  NimBLEDevice::setMTU(64);

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
  g_data->setValue(frame, len);
  g_data->notify();
}

void sendMarker(uint32_t sampleIndex, uint32_t ms) {
  if (!g_connected || g_mark == nullptr) return;
  uint8_t p[8];
  p[0]= sampleIndex        & 0xFF; p[1]=(sampleIndex >> 8)  & 0xFF;
  p[2]=(sampleIndex >> 16) & 0xFF; p[3]=(sampleIndex >> 24) & 0xFF;
  p[4]= ms         & 0xFF;         p[5]=(ms >> 8)  & 0xFF;
  p[6]=(ms >> 16)  & 0xFF;         p[7]=(ms >> 24) & 0xFF;
  g_mark->setValue(p, 8);
  g_mark->notify();
}

}  // namespace ble_stream
