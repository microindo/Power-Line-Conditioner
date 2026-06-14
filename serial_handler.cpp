#include "serial_handler.h"
#include "storage.h"
#include "relay_handler.h"
#include "modbus_handler.h"

static void printHelp();

void initSerialCmd() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== Power Line Conditioner v1.0 ===");
  Serial.println("Ketik HELP untuk daftar perintah");
}

void processSerial() {
  if (!Serial.available()) return;

  static char buffer[128];
  static size_t pos = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (pos == 0) continue;
      buffer[pos] = 0;
      pos = 0;

      char cmd[16], key[64], val[64];
      key[0] = 0;
      val[0] = 0;

      int n = sscanf(buffer, "%s %s %[^\n]", cmd, key, val);

      for (int i = 0; cmd[i]; i++) cmd[i] = toupper(cmd[i]);

      if (strcmp(cmd, "HELP") == 0) {
        printHelp();
      }
      else if (strcmp(cmd, "STATUS") == 0) {
        printConfig();
        printRelayStatus();
        printKontaktorStatus();
        Serial.print("PZEM Input: ");
        if (pzemInput.valid) {
          Serial.print(pzemInput.voltage); Serial.print("V ");
          Serial.print(pzemInput.current); Serial.print("A ");
          Serial.print(pzemInput.power); Serial.print("W ");
          Serial.print(pzemInput.energy); Serial.println("Wh");
        } else {
          Serial.println("(tidak terhubung)");
        }
        Serial.print("PZEM Output: ");
        if (pzemOutput.valid) {
          Serial.print(pzemOutput.voltage); Serial.print("V ");
          Serial.print(pzemOutput.current); Serial.print("A ");
          Serial.print(pzemOutput.power); Serial.print("W ");
          Serial.print(pzemOutput.energy); Serial.println("Wh");
        } else {
          Serial.println("(tidak terhubung)");
        }
        Serial.print("WiFi: "); Serial.println(WiFi.status() == WL_CONNECTED ? "Terhubung" : "Putus");
        Serial.print("MQTT: "); Serial.println(mqttConnected() ? "Terhubung" : "Putus");
        Serial.print("Uptime: "); Serial.print(millis() / 1000); Serial.println(" detik");
      }
      else if (strcmp(cmd, "SET") == 0) {
        if (n < 2) { Serial.println("Gunakan: SET <KEY> <VALUE>"); break; }
        for (int i = 0; key[i]; i++) key[i] = toupper(key[i]);

        if (strcmp(key, "SSID") == 0 && n >= 3) {
          strcpy(appConfig.wifi_ssid, val);
          Serial.print("SSID diubah: "); Serial.println(appConfig.wifi_ssid);
        }
        else if (strcmp(key, "PASSWORD") == 0 && n >= 3) {
          strcpy(appConfig.wifi_pass, val);
          Serial.println("Password WiFi diubah");
        }
        else if (strcmp(key, "BROKER") == 0 && n >= 3) {
          strcpy(appConfig.mqtt_host, val);
          Serial.print("Broker diubah: "); Serial.println(appConfig.mqtt_host);
        }
        else if (strcmp(key, "PORT") == 0 && n >= 3) {
          appConfig.mqtt_port = (uint16_t)atoi(val);
          Serial.print("Port diubah: "); Serial.println(appConfig.mqtt_port);
        }
        else if (strcmp(key, "BROKER_USER") == 0 && n >= 3) {
          strcpy(appConfig.mqtt_user, val);
          Serial.print("User broker diubah: "); Serial.println(appConfig.mqtt_user);
        }
        else if (strcmp(key, "BROKER_PASS") == 0 && n >= 3) {
          strcpy(appConfig.mqtt_pass, val);
          Serial.println("Password broker diubah");
        }
        else if (strcmp(key, "DELAY") == 0 && n >= 3) {
          int d = atoi(val);
          if (d >= 100 && d <= 60000) {
            appConfig.send_delay = d;
            Serial.print("Send delay diubah: "); Serial.print(appConfig.send_delay); Serial.println(" ms");
          } else {
            Serial.println("Delay 100-60000 ms");
          }
        }
        else if (strcmp(key, "RELAY") == 0 && n >= 3) {
          int idx, state;
          if (sscanf(val, "%d %d", &idx, &state) == 2 && idx >= 1 && idx <= 4) {
            setRelay(idx - 1, state == 1);
            Serial.print("Relay "); Serial.print(idx); Serial.println(state == 1 ? " ON" : " OFF");
          } else {
            Serial.println("Gunakan: SET RELAY <1-4> <0/1>");
          }
        }
        else {
          Serial.println("Key tidak dikenal. Lihat HELP.");
        }
      }
      else if (strcmp(cmd, "SAVE") == 0) {
        saveConfig();
        Serial.println("Konfigurasi tersimpan");
      }
      else if (strcmp(cmd, "RESET") == 0) {
        char arg[16] = {0};
        sscanf(buffer, "%s %s", cmd, arg);
        for (int i = 0; arg[i]; i++) arg[i] = toupper(arg[i]);

        if (strcmp(arg, "FACTORY") == 0) {
          Serial.println("FACTORY RESET...");
          resetConfig();
          for (int i = 0; i < 4; i++) setRelay(i, false);
          Serial.println("Factory reset selesai. Restart...");
          delay(500);
          ESP.restart();
        } else {
          resetConfig();
          Serial.println("Konfigurasi direset ke default");
          Serial.println("Gunakan: RESET FACTORY untuk reset penuh + restart");
        }
      }
      else if (strcmp(cmd, "RESTART") == 0) {
        Serial.println("Restart...");
        delay(100);
        ESP.restart();
      }
      else {
        Serial.print("Perintah tidak dikenal: "); Serial.println(cmd);
        Serial.println("Ketik HELP untuk daftar perintah");
      }
    }
    else {
      if (pos < sizeof(buffer) - 1) {
        buffer[pos++] = c;
      }
    }
  }
}

static void printHelp() {
  Serial.println();
  Serial.println("=== Daftar Perintah ===");
  Serial.println("HELP                      - Tampilkan ini");
  Serial.println("STATUS                    - Tampilkan semua status & konfigurasi");
  Serial.println("SET SSID <nama>           - Set SSID WiFi");
  Serial.println("SET PASSWORD <pass>       - Set password WiFi");
  Serial.println("SET BROKER <host>         - Set host MQTT broker");
  Serial.println("SET PORT <port>           - Set port MQTT broker");
  Serial.println("SET BROKER_USER <user>    - Set username MQTT");
  Serial.println("SET BROKER_PASS <pass>    - Set password MQTT");
  Serial.println("SET DELAY <ms>            - Set delay kirim data (100-60000)");
  Serial.println("SET RELAY <1-4> <0/1>     - Kontrol relay");
  Serial.println("SAVE                      - Simpan konfigurasi ke NVS");
  Serial.println("RESET                     - Reset konfigurasi ke default");
  Serial.println("RESET FACTORY             - Factory reset + matikan relay + restart");
  Serial.println("RESTART                   - Restart ESP32");
  Serial.println();
}
