/*
  Power Line Conditioner
  ESP32 + 2x PZEM-017 (Modbus RS485) + 4 Relay + 4 Kontaktor
  MQTT + Serial Monitor Configuration
*/

#include "config.h"
#include "storage.h"
#include "relay_handler.h"
#include "modbus_handler.h"
#include "mqtt_handler.h"
#include "serial_handler.h"

static unsigned long lastPZEMRead = 0;
const unsigned long pzemInterval = 2000;

void setup() {
  initSerialCmd();
  loadConfig();
  initRelay();
  initModbus();
  initMQTT();

  // Watchdog - enable task watchdog
  esp_task_wdt_init(10, true);
  esp_task_wdt_add(NULL);

  Serial.println("System ready.");
}

void loop() {
  unsigned long now = millis();

  // Baca PZEM setiap 2 detik
  if (now - lastPZEMRead >= pzemInterval) {
    lastPZEMRead = now;
    readPZEM(PZEM_ADDR_INPUT, pzemInput);
    readPZEM(PZEM_ADDR_OUTPUT, pzemOutput);
  }

  readKontaktor();
  processSerial();
  mqttLoop();

  esp_task_wdt_reset();
}
