# Power Line Conditioner

Monitoring dan kontrol power line menggunakan ESP32 dengan 2 sensor PZEM-017, 4 relay, dan 4 input kontaktor.

## Fitur

- **2x PZEM-017** — Mengukur tegangan, arus, daya, energi di sisi input dan output.
- **4 Relay** — Kontrol output via MQTT atau Serial Monitor.
- **4 Input Kontaktor** — Membaca status ON/OFF kontaktor dengan debouncing.
- **MQTT** — Kirim data dan terima perintah via broker cloud.
- **Serial Monitor** — Konfigurasi parameter tanpa upload ulang.
- **Konfigurasi Persisten** — Semua parameter tersimpan di NVS (tahan restart).
- **Heartbeat** — Kirim status device setiap 5 menit.
- **Watchdog** — Task watchdog untuk reliabilitas.

## Pin Assignment

| ESP32 | Komponen               |
|-------|------------------------|
| GPIO16| MAX485 RO (RX)         |
| GPIO17| MAX485 DI (TX)         |
| GPIO4 | MAX485 DE/RE           |
| GPIO26| Relay 1                |
| GPIO27| Relay 2                |
| GPIO32| Relay 3                |
| GPIO33| Relay 4                |
| GPIO34| Kontaktor 1            |
| GPIO35| Kontaktor 2            |
| GPIO36| Kontaktor 3            |
| GPIO39| Kontaktor 4            |
| GPIO2 | LED Indikator          |

## Software Requirements

- Arduino IDE / PlatformIO
- Board: ESP32 Arduino Core
- Libraries:
  - `ModbusMaster` — via Library Manager
  - `PubSubClient` — via Library Manager
  - `ArduinoJson` — via Library Manager

## Serial Commands

| Perintah                       | Fungsi                  |
|--------------------------------|-------------------------|
| `HELP`                         | Tampilkan bantuan       |
| `STATUS`                       | Tampilkan semua status  |
| `SET SSID <nama>`              | Set WiFi SSID           |
| `SET PASSWORD <pass>`          | Set WiFi password       |
| `SET BROKER <host>`            | Set MQTT broker host    |
| `SET PORT <port>`              | Set MQTT port           |
| `SET BROKER_USER <user>`       | Set MQTT username       |
| `SET BROKER_PASS <pass>`       | Set MQTT password       |
| `SET DELAY <ms>`               | Set delay kirim data    |
| `SET RELAY <1-4> <0/1>`        | Kontrol relay           |
| `SAVE`                         | Simpan ke NVS           |
| `RESET`                        | Reset konfigurasi       |
| `RESTART`                      | Restart ESP32           |

## MQTT Topics

### Publish
| Topic                    | Payload                                |
|--------------------------|----------------------------------------|
| `plc/data`               | Pembacaan PZEM input & output           |
| `plc/status/relay`       | Status relay 1-4                        |
| `plc/status/contactor`   | Status kontaktor 1-4                    |
| `plc/heartbeat`          | Uptime, RSSI, heap, versi               |
| `plc/online`             | LWT online/offline                      |

### Subscribe
| Topic                    | Payload      | Fungsi             |
|--------------------------|-------------|-------------------|
| `plc/relay/1/set`        | ON / OFF    | Kontrol relay 1    |
| `plc/relay/2/set`        | ON / OFF    | Kontrol relay 2    |
| `plc/relay/3/set`        | ON / OFF    | Kontrol relay 3    |
| `plc/relay/4/set`        | ON / OFF    | Kontrol relay 4    |
| `plc/config/wifi_ssid`   | string      | Set WiFi SSID      |
| `plc/config/wifi_pass`   | string      | Set WiFi password  |
| `plc/config/mqtt_host`   | string      | Set broker host    |
| `plc/config/mqtt_port`   | number      | Set broker port    |
| `plc/config/mqtt_user`   | string      | Set broker user    |
| `plc/config/mqtt_pass`   | string      | Set broker pass    |
| `plc/config/send_delay`  | number      | Set delay kirim    |
