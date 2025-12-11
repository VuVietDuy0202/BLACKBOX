#include <Arduino.h>
#include "ble_ota.h"

extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_partition.h"
}
#define BOOT_PIN 0

void TaskBootButton(void *pvParameters) {
  pinMode(BOOT_PIN, INPUT_PULLUP);
  uint16_t pressTime = 0;
  const uint16_t PRESS_THRESHOLD = 600; // 600 * 10ms = 6000ms

  const esp_partition_t* ota_0 = esp_partition_find_first(
      ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr
  );
  while (1) {
    if (digitalRead(BOOT_PIN) == LOW) {
      pressTime++;
      Serial.printf("Nút BOOT đang được nhấn: %d ms\n", pressTime * 10);
      if (pressTime >= PRESS_THRESHOLD) {
        Serial.println("Switching to OTA mode...");

        if (ota_0 && esp_ota_set_boot_partition(ota_0) == ESP_OK) {
          Serial.println("Đặt boot = OTA_0. Restart...");
          vTaskDelay(pdMS_TO_TICKS(1000));
          esp_restart();
        }
        pressTime = 0;
      }
    } else {
      pressTime = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
void setup() {
  Serial.begin(115200);
 unsigned long start = millis();
  while (!Serial ) {
        delay(100);
  }
  Serial.println("\n\n--------------------------------");
  Serial.println("Factory App đang khởi động...");
  Serial.println("--------------------------------");
  ble_ota_begin("DUY");
  xTaskCreate(TaskBootButton, "BootButtonTask", 4096, NULL, 1, NULL);
}

void loop() {
  // Không cần code gì trong loop
}
