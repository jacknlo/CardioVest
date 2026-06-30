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

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(&g_serverCb);

  NimBLEService* svc = server->createService(SVC_UUID);
  g_data = svc->createCharacteristic(DATA_UUID, NIMBLE_PROPERTY::NOTIFY);
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

}  // namespace ble_stream
