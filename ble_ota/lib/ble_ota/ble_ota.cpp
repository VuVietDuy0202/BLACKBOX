#include <Arduino.h>
#include <NimBLEDevice.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "ble_ota.h"

#define SERVICE_UUID        "0d155824-70ec-4477-bde8-bd92fb593343"
#define CHARACTERISTIC_UUID "51adc254-f484-46db-97d6-d6037473cfc7"

static bool deviceConnected = false;
static bool otaStarted = false;
static const esp_partition_t* ota_partition = nullptr;
static esp_ota_handle_t ota_handle = 0;
static size_t total_received = 0;

class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    deviceConnected = true;
    Serial.printf(" Kết nối BLE từ: %s\n", connInfo.getAddress().toString().c_str());
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    deviceConnected = false;
    otaStarted = false;
    total_received = 0;
    ota_handle = 0;
    ota_partition = nullptr;
    Serial.println(" Mất kết nối BLE. Quảng bá lại...");
    NimBLEDevice::startAdvertising();
  }
};
class MyCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    const uint8_t* data = pChar->getValue().data();
    size_t length = pChar->getLength();
    if (length == 0) return;
    if (length == 9 && memcmp(data, "OTA_BEGIN", 9) == 0) {
      if (otaStarted) return;

      ota_partition = esp_ota_get_next_update_partition(NULL);
      if (!ota_partition) {
        Serial.println("[Lỗi] Không tìm thấy phân vùng OTA phù hợp!");
        return;
      }
      esp_err_t err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
      if (err != ESP_OK) {
        Serial.printf("[Lỗi] esp_ota_begin lỗi: %s\n", esp_err_to_name(err));
        return;
      }
      otaStarted = true;
      total_received = 0;
      Serial.printf("Bắt đầu OTA vào phân vùng '%s' tại offset 0x%X\n",
                    ota_partition->label, ota_partition->address);
      return;
    }
    if (length >= 3 && memcmp(data, "EOF", 3) == 0) {
      Serial.println("  Nhận EOF. Kết thúc OTA...");

      esp_err_t err = esp_ota_end(ota_handle);
      if (err != ESP_OK) {
        Serial.printf("[Lỗi] esp_ota_end lỗi: %s\n", esp_err_to_name(err));
        return;
      }

      err = esp_ota_set_boot_partition(ota_partition);
      if (err == ESP_OK) {
        Serial.println("  Đã đặt boot sang OTA. Restart...");
        delay(1000);
        esp_restart();
      } else {
        Serial.printf("[Lỗi] Không thể set boot: %s\n", esp_err_to_name(err));
      }
      return;
    }

    if (!otaStarted) {
      Serial.println("[Lỗi] Nhận dữ liệu nhưng chưa gửi OTA_BEGIN!");
      return;
    }

    esp_err_t err = esp_ota_write(ota_handle, data, length);
    if (err != ESP_OK) {
      Serial.printf("[Lỗi] Ghi lỗi: %s\n", esp_err_to_name(err));
      return;
    }

    total_received += length;
    Serial.printf("[Dữ liệu] Ghi %d bytes (tổng: %d)\n", length, total_received);

    // Gửi phản hồi "OK"
    pChar->setValue("OK");
    pChar->notify();
  }
};

void ble_ota_begin(const char *device_name) {
  //Serial.begin(115200);
  Serial.println("Factory App khởi động");
  NimBLEDevice::init(device_name);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  NimBLECharacteristic* pChar = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
  );
  pChar->setCallbacks(new MyCallbacks());
  pChar->setValue("ESP32 Ready for OTA");
  //pChar->createDescriptor("2902", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

  pService->start();

  NimBLEAdvertisementData advData;
  NimBLEAdvertisementData scanResp;
  advData.setCompleteServices(NimBLEUUID(SERVICE_UUID));
 // advData.setName(device_name);
  scanResp.setName(device_name);
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponseData(scanResp);
  pAdvertising->start();

  Serial.println(" BLE OTA sẵn sàng. Gửi file và kết thúc bằng EOF");
}
