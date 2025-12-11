#ifndef BLE_OTA_H
#define BLE_OTA_H

#include <Arduino.h>
void ble_ota_begin(const char *device_name);
void ble_ota_loop(void);
#endif
