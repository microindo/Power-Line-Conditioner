# Power Line Conditioner

Sistem monitoring dan kontrol power line menggunakan **ESP32** dengan **2 sensor PZEM-017** (input & output), **4 relay output**, dan **4 input kontaktor**. Komunikasi menggunakan **MQTT** untuk IoT cloud dan **Serial Monitor** untuk konfigurasi lokal.

Dibuat oleh: [microindo](https://github.com/microindo)

---

## Daftar Isi

1. [Fitur](#fitur)
2. [Komponen & Alat](#komponen--alat)
3. [Skema Rangkaian](#skema-rangkaian)
4. [Instalasi Software](#instalasi-software)
5. [Konfigurasi Alamat PZEM-017](#konfigurasi-alamat-pzem-017)
6. [Upload Program](#upload-program)
7. [Konfigurasi Awal via Serial Monitor](#konfigurasi-awal-via-serial-monitor)
8. [Daftar Perintah Serial](#daftar-perintah-serial)
9. [MQTT Topics & Payload](#mqtt-topics--payload)
10. [Cara Pengetesan Sistem](#cara-pengetesan-sistem)
11. [Troubleshooting](#troubleshooting)
12. [Struktur File](#struktur-file)
13. [Pengembangan](#pengembangan)

---

## Fitur

| Fitur | Keterangan |
|-------|-----------|
| **2x PZEM-017** | Mengukur tegangan (V), arus (A), daya (W), energi (Wh) di sisi **input** dan **output** secara real-time. |
| **4 Relay** | Kontrol 4 output relay independen via MQTT atau Serial Monitor. |
| **4 Input Kontaktor** | Membaca status ON/OFF dari 4 kontaktor dengan **debouncing software 50ms**. |
| **MQTT Cloud** | Kirim data periodik dan terima perintah kontrol via MQTT broker. |
| **Serial Monitor** | Konfigurasi semua parameter tanpa perlu upload ulang firmware. |
| **Konfigurasi Persisten** | Semua parameter tersimpan di **NVS Preferences** — tetap awet walau restart / mati listrik. |
| **Heartbeat** | Mengirim status device (uptime, RSSI, heap, versi) setiap **5 menit**. |
| **Watchdog** | **ESP32 Task Watchdog Timer (TWDT)** — reset otomatis jika system hang. |
| **Auto Reconnect** | WiFi dan MQTT auto reconnect jika koneksi terputus. |
| **Last Will (LWT)** | Broker mendapat notifikasi `offline` jika ESP32 mati mendadak. |

---

## Komponen & Alat

### Hardware

| No | Komponen | Spesifikasi | Jumlah |
|----|----------|-------------|--------|
| 1 | ESP32 Board | DOIT ESP32 DEVKIT V1 / NodeMCU-32S | 1 |
| 2 | PZEM-017 | DC Energy Meter (Modbus RTU) | 2 |
| 3 | MAX485 | RS485 to TTL Converter | 1 |
| 4 | Relay Module | 4-Channel Relay 5V (Active LOW) | 1 |
| 5 | Power Supply | 5V / 2A untuk ESP32 | 1 |
| 6 | Power Supply | 12V / 1A untuk relay (jika perlu) | 1 |
| 7 | Breadboard | 830 point | 1 |
| 8 | Kabel Jumper | Male-to-Male, Male-to-Female | ~30 |
| 9 | Push Button | Tactile switch (untuk test kontaktor) | 4 |
| 10 | Resistor | 10kΩ (pull-up untuk input kontaktor) | 4 |
| 11 | Kabel Micro USB | Data + Power | 1 |

### Software

| Software | Fungsi |
|----------|--------|
| Arduino IDE 2.x | Editor dan compiler firmware |
| ESP32 Arduino Core | Board support package |
| ModbusMaster Library | Komunikasi Modbus RTU via RS485 |
| PubSubClient Library | MQTT client untuk ESP32 |
| ArduinoJson Library | Membuat dan parsing JSON payload |

### Alat Ukur (untuk testing)

| Alat | Fungsi |
|------|--------|
| Multimeter digital | Verifikasi tegangan dan kontinuitas |
| Power supply DC variabel | Simulasi beban untuk test PZEM-017 |
| MQTT Client (MQTTX / Mosquitto) | Test publish & subscribe manual |
| USB-to-RS485 Converter | Konfigurasi alamat PZEM-017 |

---

## Skema Rangkaian

### Diagram Blok

```
                   +------------------+
                   |    Power Supply   |
                   |     (5V / 12V)    |
                   +--------+---------+
                            |
                    +-------+--------+
                    |     ESP32       |
                    |                 |
                    |  GPIO16 <-------> RO MAX485
                    |  GPIO17 <-------> DI MAX485
                    |  GPIO4  <-------> DE/RE MAX485
                    |                 |
                    |  GPIO26 --------> Relay 1
                    |  GPIO27 --------> Relay 2
                    |  GPIO32 --------> Relay 3
                    |  GPIO33 --------> Relay 4
                    |                 |
                    |  GPIO34 <-------- Kontaktor 1
                    |  GPIO35 <-------- Kontaktor 2
                    |  GPIO36 <-------- Kontaktor 3
                    |  GPIO39 <-------- Kontaktor 4
                    |                 |
                    |  GPIO2  --------> LED Indikator
                    +-------+---------+
                            |
                   +--------+---------+
                   |     MAX485        |
                   |  A(+) ------> PZEM-017 Input (A)
                   |  B(-) ------> PZEM-017 Input (B)
                   |  A(+) ------> PZEM-017 Output (A)
                   |  B(-) ------> PZEM-017 Output (B)
                   +------------------+
```

### Wiring Detail

#### MAX485 ke ESP32

| MAX485 | ESP32 |
|--------|-------|
| RO (Receiver Out) | GPIO16 (UART2 RX) |
| DI (Driver Input) | GPIO17 (UART2 TX) |
| DE (Driver Enable) | GPIO4 (gabung dengan RE) |
| RE (Receiver Enable) | GPIO4 (gabung dengan DE) |
| VCC | 5V |
| GND | GND |
| A (RS485+) | PZEM-017 (A) |
| B (RS485-) | PZEM-017 (B) |

#### Relay ke ESP32

| Relay Module | ESP32 |
|-------------|-------|
| IN1 | GPIO26 |
| IN2 | GPIO27 |
| IN3 | GPIO32 |
| IN4 | GPIO33 |
| VCC | 5V (atau eksternal) |
| GND | GND (satu ground dengan ESP32) |

> **Catatan:** Jika relay module menggunakan **Active LOW**, maka `HIGH` = relay OFF, `LOW` = relay ON. Program ini sudah menangani hal tersebut.

#### Kontaktor (Push Button untuk Simulasi) ke ESP32

| Kontaktor / Button | ESP32 |
|--------------------|-------|
| Kontaktor 1 | GPIO34 → 10kΩ → GND (pull-down) |
| Kontaktor 2 | GPIO35 → 10kΩ → GND (pull-down) |
| Kontaktor 3 | GPIO36 → 10kΩ → GND (pull-down) |
| Kontaktor 4 | GPIO39 → 10kΩ → GND (pull-down) |
| Common | 3.3V |

**Cara kerja:** Button ditekan → GPIO ke HIGH → kontaktor status ON.

#### PZEM-017 ke MAX485 (RS485 Bus)

| PZEM-017 | MAX485 |
|----------|--------|
| A (+) | A (+) |
| B (-) | B (-) |

Kedua PZEM-017 terhubung ke **bus RS485 yang sama** (paralel). Perbedaan alamat Modbus yang membedakan keduanya.

---

## Instalasi Software

### Langkah 1: Install Arduino IDE

1. Download Arduino IDE dari https://www.arduino.cc/en/software
2. Install sesuai OS (Windows/Linux/Mac)
3. Buka Arduino IDE

### Langkah 2: Install Board ESP32

1. Buka **File > Preferences**
2. Pada **Additional Boards Manager URLs**, isi:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Klik **OK**
4. Buka **Tools > Board > Boards Manager**
5. Cari **"ESP32"**
6. Klik **Install** pada **"ESP32 by Espressif Systems"**
7. Tunggu hingga selesai
8. Pilih board: **Tools > Board > ESP32 Arduino > ESP32 Dev Module**

### Langkah 3: Install Libraries

Buka **Sketch > Include Library > Manage Libraries**, lalu cari dan install satu per satu:

| Library | Cari | Versi Min | Fungsi |
|---------|------|-----------|--------|
| **ModbusMaster** | `modbusmaster` by 4-20ma | ≥ 2.0 | Komunikasi Modbus RTU |
| **PubSubClient** | `pubsubclient` by Nick O'Leary | ≥ 2.8 | MQTT client |
| **ArduinoJson** | `arduinojson` by Benoit Blanchon | ≥ 6.18 | Parsing & generate JSON |

**Cara install library:**
1. Buka **Sketch > Include Library > Manage Libraries**
2. Ketik nama library di kolom pencarian
3. Pilih library yang benar
4. Klik **Install**

### Langkah 4: Download Source Code

Clone atau download repository ini:

```bash
git clone https://github.com/microindo/Power-Line-Conditioner.git
```

Atau download ZIP dari halaman GitHub, lalu ekstrak.

Buka folder hasil download, lalu buka file **Power_Line_Conditioner.ino** di Arduino IDE.

---

## Konfigurasi Alamat PZEM-017

PZEM-017 memiliki alamat Modbus default `0x01`. Karena kita menggunakan **2 unit**, alamat salah satu harus diubah menjadi `0x02`.

### Metode 1: Menggunakan Software PC

1. Siapkan **USB-to-RS485 converter**
2. Hubungkan converter ke PZEM-017 pertama (yang akan diubah alamatnya) — **lepaskan PZEM-017 kedua dari bus**
3. Jalankan software konfigurasi PZEM-017 (dari Peacefair atau tool Python pihak ketiga)
4. Ubah alamat dari `0x01` menjadi `0x02`
5. Verifikasi perubahan
6. Ulangi untuk PZEM-017 kedua jika perlu (pastikan kembali ke `0x01`)

### Metode 2: Menggunakan ESP32 (Inisialisasi Satu Kali)

Gunakan kode di bawah untuk mengubah alamat. Upload sekali, jalankan, lalu upload ulang program utama.

```cpp
#include <ModbusMaster.h>

ModbusMaster node;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Ganti 0x01 menjadi 0x02
  node.begin(0x01, Serial2);
  uint16_t addr[1] = {0x0002};
  node.writeSingleRegister(0x0002, addr[0]);  // Register alamat

  // Verifikasi
  node.begin(0x02, Serial2);
  uint8_t res = node.readHoldingRegisters(0x0002, 1);
  if (res == node.ku8MBSuccess) {
    Serial.print("Alamat baru: 0x");
    Serial.println(node.getResponseBuffer(0), HEX);
  }
}

void loop() {}
```

> **PENTING:** Setelah mengubah alamat, beri label fisik pada PZEM-017 yang sudah diubah agar tidak tertukar.

### Konfigurasi di Code

Alamat sudah diatur di `config.h`:

```cpp
#define PZEM_ADDR_INPUT   0x01   // PZEM di sisi input
#define PZEM_ADDR_OUTPUT  0x02   // PZEM di sisi output
```

Sesuaikan jika alamat yang kamu set berbeda.

---

## Upload Program

### Langkah-langkah

1. Buka **Power_Line_Conditioner.ino** di Arduino IDE
2. **Jangan ubah kode** — semua konfigurasi dilakukan via Serial/MQTT setelah upload
3. Pastikan board yang dipilih: **Tools > Board > ESP32 Dev Module**
4. Pilih port yang sesuai: **Tools > Port > COMx** (Windows) atau **/dev/ttyUSB0** (Linux)
5. Klik tombol **Upload** (→)
6. Jika gagal, coba tahan tombol **BOOT** di ESP32 saat mulai upload, lepas setelah selesai
7. Setelah upload selesai, buka **Tools > Serial Monitor**
8. Set **Baud Rate** ke **115200**
9. ESP32 akan mereset dan menampilkan pesan:

```
=== Power Line Conditioner v1.0 ===
Ketik HELP untuk daftar perintah
System ready.
```

### Catatan Penting

| Masalah Upload | Solusi |
|----------------|--------|
| `A fatal error occurred: Failed to connect to ESP32` | Tahan **BOOT** saat mulai upload |
| `Timed out waiting for packet header` | Cabut-tancap USB atau tekan **EN/RST** |
| No Serial port muncul | Install driver CP210x atau CH340 |

---

## Konfigurasi Awal via Serial Monitor

Setelah upload berhasil, lakukan konfigurasi awal:

### 1. Set WiFi

```
SET SSID Rumah_Saya
SET PASSWORD abc12345
SAVE
RESTART
```

ESP32 akan restart dan mencoba konek WiFi.

### 2. Set MQTT Broker

```
SET BROKER broker.hivemq.cloud
SET PORT 8883
SET BROKER_USER user_anda
SET BROKER_PASS pass_anda
SAVE
RESTART
```

> **Catatan:** Untuk port `8883` (MQTTS), pastikan library PubSubClient mendukung TLS. Jika tidak, gunakan port `1883` untuk koneksi non-TLS.

### 3. Verifikasi Koneksi

Setelah restart, ketik:

```
STATUS
```

Output yang diharapkan:

```
--- Konfigurasi ---
SSID: Rumah_Saya
Password: ***
Broker: broker.hivemq.cloud
Port: 1883
...

WiFi: Terhubung
MQTT: Terhubung
...
```

---

## Daftar Perintah Serial

### Perintah Konfigurasi

| Perintah | Contoh | Deskripsi |
|----------|--------|-----------|
| `SET SSID <nama>` | `SET SSID Rumah_Saya` | Set nama WiFi |
| `SET PASSWORD <pass>` | `SET PASSWORD rahasia` | Set password WiFi |
| `SET BROKER <host>` | `SET BROKER test.mosquitto.org` | Set host MQTT broker |
| `SET PORT <port>` | `SET PORT 1883` | Set port MQTT |
| `SET BROKER_USER <user>` | `SET BROKER_USER admin` | Set username MQTT |
| `SET BROKER_PASS <pass>` | `SET BROKER_PASS admin123` | Set password MQTT |
| `SET DELAY <ms>` | `SET DELAY 5000` | Set interval kirim data (100-60000ms) |

### Perintah Kontrol

| Perintah | Contoh | Deskripsi |
|----------|--------|-----------|
| `SET RELAY <1-4> <0/1>` | `SET RELAY 1 1` | Relay 1 ON (1) / OFF (0) |

### Perintah Sistem

| Perintah | Deskripsi |
|----------|-----------|
| `HELP` | Tampilkan daftar perintah |
| `STATUS` | Tampilkan semua konfigurasi, status, dan pembacaan sensor |
| `SAVE` | Simpan konfigurasi ke NVS (tahan restart) |
| `RESET` | Reset semua konfigurasi ke default |
| `RESTART` | Restart ESP32 |

### Contoh Sesi Serial

```
=== Power Line Conditioner v1.0 ===
Ketik HELP untuk daftar perintah

> SET RELAY 1 1
Relay 1 ON

> SET RELAY 2 1
Relay 2 ON

> STATUS
--- Konfigurasi ---
SSID: Rumah_Saya
Password: ***
Broker: broker.hivemq.cloud
Port: 1883
...
Relay: R1=ON R2=ON R3=OFF R4=OFF
Kontaktor: C1=ON C2=OFF C3=OFF C4=OFF
PZEM Input: 24.10V 0.50A 12.05W 0.00Wh
PZEM Output: 24.08V 0.48A 11.56W 0.00Wh
WiFi: Terhubung
MQTT: Terhubung
Uptime: 360 detik

> SAVE
Konfigurasi tersimpan
```

---

## MQTT Topics & Payload

### Topik Publish (ESP32 → Cloud)

#### `plc/data` — Data PZEM

Dikirim setiap `send_delay` ms (default 5 detik).

```json
{
  "v_in": 24.12,
  "i_in": 0.50,
  "p_in": 12.06,
  "e_in": 0.05,
  "v_out": 24.08,
  "i_out": 0.48,
  "p_out": 11.56,
  "e_out": 0.03,
  "input_valid": true,
  "output_valid": true
}
```

| Field | Tipe | Satuan | Deskripsi |
|-------|------|--------|-----------|
| `v_in` | float | Volt | Tegangan input |
| `i_in` | float | Ampere | Arus input |
| `p_in` | float | Watt | Daya input |
| `e_in` | float | Wh | Energi input (akumulasi) |
| `v_out` | float | Volt | Tegangan output |
| `i_out` | float | Ampere | Arus output |
| `p_out` | float | Watt | Daya output |
| `e_out` | float | Wh | Energi output (akumulasi) |
| `input_valid` | bool | - | `true` jika Modbus sukses |
| `output_valid` | bool | - | `true` jika Modbus sukses |

#### `plc/status/relay` — Status Relay

```json
{"r1":"ON","r2":"OFF","r3":"ON","r4":"OFF"}
```

#### `plc/status/contactor` — Status Kontaktor

```json
{"c1":"ON","c2":"OFF","c3":"ON","c4":"OFF"}
```

#### `plc/heartbeat` — Heartbeat (setiap 5 menit)

```json
{
  "uptime": 600,
  "rssi": -65,
  "heap": 198000,
  "ver": "1.0"
}
```

| Field | Tipe | Satuan | Deskripsi |
|-------|------|--------|-----------|
| `uptime` | int | detik | Waktu sejak ESP32 menyala |
| `rssi` | int | dBm | Kekuatan sinyal WiFi |
| `heap` | int | bytes | Sisa RAM bebas |
| `ver` | string | - | Versi firmware |

#### `plc/online` — Last Will & Testament

- **Publish saat konek:** `online` (retained)
- **LWT (jika putus):** `offline` (retained)

Cocok untuk monitor apakah device masih hidup.

### Topik Subscribe (Cloud → ESP32)

#### Kontrol Relay

| Topik | Payload | Efek |
|-------|---------|------|
| `plc/relay/1/set` | `ON` atau `OFF` | Relay 1 ON/OFF |
| `plc/relay/2/set` | `ON` atau `OFF` | Relay 2 ON/OFF |
| `plc/relay/3/set` | `ON` atau `OFF` | Relay 3 ON/OFF |
| `plc/relay/4/set` | `ON` atau `OFF` | Relay 4 ON/OFF |

Contoh:
```bash
mosquitto_pub -h broker.hivemq.cloud -t "plc/relay/1/set" -m "ON"
```

#### Konfigurasi via MQTT

| Topik | Payload | Efek |
|-------|---------|------|
| `plc/config/wifi_ssid` | `"Rumah_Saya"` | Ubah SSID (tersimpan ke NVS) |
| `plc/config/wifi_pass` | `"password123"` | Ubah pass WiFi (tersimpan ke NVS) |
| `plc/config/mqtt_host` | `"broker.hivemq.cloud"` | Ubah host broker |
| `plc/config/mqtt_port` | `"1883"` | Ubah port broker |
| `plc/config/mqtt_user` | `"admin"` | Ubah username broker |
| `plc/config/mqtt_pass` | `"admin123"` | Ubah password broker |
| `plc/config/send_delay` | `"10000"` | Ubah delay kirim data |

> **Catatan:** Perubahan konfigurasi via MQTT akan tersimpan otomatis ke NVS.

---

## Cara Pengetesan Sistem

### Tahap 1: Power-On Test

**Tujuan:** Memastikan ESP32 menyala dan program berjalan.

1. Hubungkan ESP32 ke power supply / USB
2. Buka **Serial Monitor** (baud 115200)
3. Cek pesan:
   ```
   === Power Line Conditioner v1.0 ===
   System ready.
   ```
4. Jika tidak muncul, tekan tombol **EN/RST** di ESP32
5. Ketik `HELP` → pastikan daftar perintah tampil

**Kriteria sukses:** Serial Monitor menampilkan pesan dan merespon perintah.

### Tahap 2: Test Serial Commands

**Tujuan:** Memastikan semua perintah serial berfungsi.

| Langkah | Perintah | Hasil yang Diharapkan |
|---------|----------|----------------------|
| 1 | `STATUS` | Menampilkan semua konfigurasi dan status |
| 2 | `SET RELAY 1 1` | `Relay 1 ON`, relay di pin 26 menyala |
| 3 | `SET RELAY 1 0` | `Relay 1 OFF`, relay mati |
| 4 | `SET DELAY 10000` | `Send delay diubah: 10000 ms` |
| 5 | `SET DELAY 50` | `Delay 100-60000 ms` (validasi bekerja) |
| 6 | `SAVE` | `Konfigurasi tersimpan` |
| 7 | `RESTART` | ESP32 restart, konfigurasi delay 10000 masih tersimpan |

**Kriteria sukses:** Semua perintah merespon sesuai.

### Tahap 3: Test PZEM-017 (Modbus)

**Tujuan:** Memastikan komunikasi Modbus dengan kedua PZEM-017 berhasil.

**Prasyarat:** MAX485 sudah terhubung ke kedua PZEM-017.

1. Pastikan PZEM-017 mendapat power (12V dari sumber DC)
2. Buka Serial Monitor
3. Ketik `STATUS`
4. Cek baris:
   ```
   PZEM Input: 24.12V 0.50A 12.06W 0.05Wh
   PZEM Output: 24.08V 0.48A 11.56W 0.03Wh
   ```

**Jika gagal:**
```
PZEM Input: (tidak terhubung)
```

**Kemungkinan penyebab:**
- MAX485 tidak terhubung ke bus RS485 dengan benar
- Kabel A/B terbalik
- Alamat PZEM-017 tidak sesuai (bukan 0x01 dan 0x02)
- Power PZEM-017 tidak aktif
- Terminasi resistor 120Ω tidak dipasang (untuk jarak jauh)

**Kriteria sukses:** Kedua PZEM menampilkan data yang valid.

### Tahap 4: Test Relay

**Tujuan:** Memastikan 4 relay dapat dikontrol.

**Peralatan:** Multimeter atau LED + resistor 220Ω.

1. Hubungkan multimeter ke pin relay 1 (COM + NO)
2. Serial: `SET RELAY 1 1`
3. Multimeter menunjukkan **kontak menutup** (continuity / 0Ω)
4. Serial: `SET RELAY 1 0`
5. Multimeter menunjukkan **kontak terbuka** (OL / tak terhubung)
6. Ulangi untuk relay 2, 3, 4

**Test simultan:**
```
SET RELAY 1 1
SET RELAY 2 1
SET RELAY 3 1
SET RELAY 4 1
STATUS
```
Output: `Relay: R1=ON R2=ON R3=ON R4=ON`

**Kriteria sukses:** Semua relay dapat ON/OFF sesuai perintah.

### Tahap 5: Test Input Kontaktor

**Tujuan:** Memastikan 4 input kontaktor terbaca.

**Peralatan:** 4x push button atau kabel jumper.

1. Hubungkan GPIO34 ke 3.3V (simulasi kontaktor ON)
2. Ketik `STATUS`
3. Cek: `Kontaktor: C1=ON C2=OFF C3=OFF C4=OFF`
4. Lepas GPIO34 dari 3.3V
5. Ketik `STATUS`
6. Cek: `Kontaktor: C1=OFF C2=OFF C3=OFF C4=OFF`
7. Ulangi untuk GPIO35, GPIO36, GPIO39

**Test bouncing:** Tekan dan lepas button cepat beberapa kali. Debouncing 50ms mencegah pembacaan ganda.

**Kriteria sukses:** Status kontaktor berubah sesuai input.

### Tahap 6: Test WiFi

**Tujuan:** Memastikan ESP32 bisa konek ke WiFi.

1. Jika belum dikonfigurasi:
   ```
   SET SSID NamaWiFi
   SET PASSWORD PasswordWiFi
   SAVE
   RESTART
   ```
2. Setelah restart, lihat Serial Monitor:
   ```
   Menghubungkan WiFi....
   WiFi terhubung
   IP: 192.168.1.100
   ```
3. Jika gagal:
   ```
   Menghubungkan WiFi....
   WiFi gagal
   ```
   Periksa SSID/password, atau jarak dari access point.

**Kriteria sukses:** ESP32 mendapat IP dan `STATUS` menunjukkan `WiFi: Terhubung`.

### Tahap 7: Test MQTT

**Tujuan:** Memastikan komunikasi MQTT berfungsi (publish & subscribe).

**Prasyarat:** WiFi sudah terhubung.

**Alat:** MQTT client (MQTTX / mosquitto_sub).

#### Test Subscribe (kontrol dari cloud)

1. Buka terminal, subscribe ke semua topik:
   ```bash
   mosquitto_sub -h broker.hivemq.cloud -t "plc/#" -v
   ```
2. ESP32 restart → akan terlihat:
   ```
   plc/online online
   plc/heartbeat {"uptime":0,"rssi":-65,"heap":198000,"ver":"1.0"}
   ```
3. Kirim perintah kontrol relay:
   ```bash
   mosquitto_pub -h broker.hivemq.cloud -t "plc/relay/1/set" -m "ON"
   ```
4. ESP32 akan merespon:
   ```
   plc/status/relay {"r1":"ON","r2":"OFF","r3":"OFF","r4":"OFF"}
   ```

#### Test Publish (ESP32 ke cloud)

Setelah `send_delay` ms, akan terlihat data di MQTT client:
```
plc/data {"v_in":24.12,"i_in":0.50,"p_in":12.06,"e_in":0.05,...}
plc/status/relay {"r1":"ON","r2":"OFF","r3":"OFF","r4":"OFF"}
plc/status/contactor {"c1":"ON","c2":"OFF","c3":"OFF","c4":"OFF"}
```

#### Test Konfigurasi via MQTT

```bash
mosquitto_pub -h broker.hivemq.cloud -t "plc/config/send_delay" -m "3000"
```
ESP32 akan menyimpan dan menggunakan delay 3 detik.

#### Test LWT (Last Will)

1. Matikan power ESP32
2. Dalam beberapa detik, MQTT client akan menerima:
   ```
   plc/online offline
   ```

**Kriteria sukses:** Semua topik publish/subscribe berfungsi dua arah.

### Tahap 8: Test Heartbeat

**Tujuan:** Memastikan heartbeat terkirim setiap 5 menit.

1. Catat waktu pertama kali ESP32 menyala
2. Tunggu 5 menit
3. Di MQTT client, akan muncul:
   ```
   plc/heartbeat {"uptime":300,"rssi":-65,"heap":195000,"ver":"1.0"}
   ```
4. Tunggu 5 menit lagi:
   ```
   plc/heartbeat {"uptime":600,"rssi":-64,"heap":193000,"ver":"1.0"}
   ```

**Kriteria sukses:** Heartbeat muncul setiap 300 detik (±2 detik).

### Tahap 9: Test Watchdog

**Tujuan:** Memastikan watchdog bekerja (ESP32 auto-reset jika hang).

**Metode simulasi:** (Hanya untuk testing — jangan di code final)

1. Tambahkan baris ini di `loop()` untuk simulasi hang:
   ```cpp
   while (true) {}  // infinite loop, watchdog akan reset
   ```
2. Upload dan tunggu
3. Setelah ~10 detik, ESP32 akan restart (terlihat di Serial Monitor)
4. Hapus baris tersebut dan upload ulang program normal

**Kriteria sukses:** ESP32 restart otomatis saat hang, tanpa intervensi manual.

### Tahap 10: Test End-to-End

**Tujuan:** Simulasi skenario dunia nyata.

1. Hubungkan beban (lampu DC / motor kecil) ke relay 1
2. Hubungkan PZEM input ke power supply 24V
3. Hubungkan PZEM output ke beban via relay
4. Nyalakan sistem

**Skenario 1 — Monitor dari jarak jauh:**
1. Buka MQTT client
2. Subscribe ke `plc/#`
3. Amati data tegangan, arus, daya muncul setiap send_delay
4. Pastikan `input_valid` dan `output_valid` = `true`

**Skenario 2 — Kontrol dari jarak jauh:**
1. Kirim `plc/relay/1/set` → `ON`
2. Beban menyala
3. Relay status berubah di `plc/status/relay`
4. PZEM output mulai membaca arus (karena beban ON)
5. Kirim `plc/relay/1/set` → `OFF`
6. Beban mati, PZEM output arus = 0

**Skenario 3 — Konfigurasi ulang dari jarak jauh:**
1. Ubah delay kirim via MQTT: `plc/config/send_delay` → `2000`
2. Data mulai terkirim setiap 2 detik
3. Matikan power ESP32
4. Nyalakan kembali
5. Data tetap terkirim setiap 2 detik (konfigurasi tersimpan di NVS)

**Kriteria sukses:** Semua skenario berjalan sesuai.

---

## Troubleshooting

### Masalah Hardware

| Masalah | Kemungkinan | Solusi |
|---------|-------------|--------|
| ESP32 tidak menyala | Power tidak cukup | Gunakan power supply 5V/2A |
| Serial Monitor tidak tampil | Port salah | Cek port di Tools > Port |
| | Board salah | Pilih ESP32 Dev Module |
| PZEM data tidak terbaca | Kabel A/B terbalik | Tukar kabel A dan B |
| | Alamat salah | Cek alamat Modbus (harus 0x01 dan 0x02) |
| | MAX485 tidak aktif | Pastikan DE/RE terhubung ke GPIO4 |
| | Power PZEM mati | PZEM-017 butuh power 12V DC |
| Relay tidak menyala | Relay active LOW | Program sudah handle, cek wiring VCC/GND |
| | Power relay kurang | Relay 5V butuh supply terpisah |
| Kontaktor tidak terbaca | Pull-up/down salah | Pastikan resistor 10kΩ terpasang |
| | Pin input salah | GPIO34-39 adalah input-only |

### Masalah Software

| Masalah | Solusi |
|---------|--------|
| Upload gagal `Failed to connect` | Tahan tombol BOOT selama upload |
| ESP32 loop restart | Ada exception, cek Serial Monitor untuk stack trace |
| WiFi tidak konek | Cek SSID/password, pastikan AP menyala |
| MQTT tidak konek | Cek host/port/user/pass, pastikan broker aktif |
| Data PZEM tidak update | Cek `input_valid` / `output_valid` di data JSON |
| Konfigurasi hilang setelah restart | Jalankan `SAVE` sebelum `RESTART` |

### Cara Debugging

1. **Cek status sistem:** `STATUS` — lihat semua parameter dan pembacaan
2. **Cek Serial output:** Pastikan baud Serial Monitor = 115200
3. **Cek error PZEM:** Lihat field `input_valid` / `output_valid` di MQTT
4. **Cek MQTT:** Subscribe ke `plc/#` untuk melihat semua traffic
5. **Cek watchdog:** Jika ESP32 restart sendiri, watchdog aktif — ada bagian code yang hang
6. **Cek heap:** Di heartbeat, jika `heap` terus menurun, ada memory leak

---

## Struktur File

```
Power Line Conditioner/
├── Power_Line_Conditioner.ino   # Main program: setup(), loop(), watchdog
├── config.h                     # Pin definitions, alamat PZEM, struct, konstanta
├── storage.h                    # Deklarasi fungsi baca/tulis NVS
├── storage.cpp                  # Implementasi loadConfig(), saveConfig(), resetConfig()
├── relay_handler.h              # Deklarasi fungsi relay & kontaktor
├── relay_handler.cpp            # Implementasi setRelay(), readKontaktor(), debounce
├── modbus_handler.h             # Deklarasi fungsi Modbus
├── modbus_handler.cpp           # Implementasi readPZEM(), initModbus()
├── mqtt_handler.h               # Deklarasi fungsi MQTT
├── mqtt_handler.cpp             # Implementasi MQTT connect, publish, subscribe, callback
├── serial_handler.h             # Deklarasi fungsi serial command
├── serial_handler.cpp           # Implementasi parser perintah serial
├── README.md                    # Dokumentasi ini
└── .gitignore
```

### Alur Program

```
Power On
  │
  ├── setup()
  │   ├── initSerialCmd()     → Serial.begin(115200)
  │   ├── loadConfig()        → Baca NVS Preferences
  │   ├── initRelay()         → Set pin mode relay & kontaktor
  │   ├── initModbus()        → Init Serial2 (UART2) untuk RS485
  │   ├── initMQTT()          → Konek WiFi & MQTT
  │   └── esp_task_wdt_init() → Start watchdog
  │
  └── loop() [setiap siklus ~10ms]
      ├── readPZEM()          → Setiap 2 detik
      ├── readKontaktor()     → Baca input kontaktor + debounce
      ├── processSerial()     → Proses input Serial Monitor
      ├── mqttLoop()          → Maintain MQTT + publish data
      └── esp_task_wdt_reset()→ Feed watchdog

MQTT Callback (async)
  ├── Kontrol relay: plc/relay/1/set .. 4/set
  └── Konfigurasi: plc/config/#
      └── saveConfig() → Simpan ke NVS
```

### Fungsi Penting

| Fungsi | File | Deskripsi |
|--------|------|-----------|
| `setup()` | .ino | Inisialisasi semua hardware dan koneksi |
| `loop()` | .ino | Main loop — baca sensor, proses perintah, maintain koneksi |
| `loadConfig()` | storage.cpp | Baca konfigurasi dari NVS ke struct `appConfig` |
| `saveConfig()` | storage.cpp | Simpan struct `appConfig` ke NVS |
| `initRelay()` | relay_handler.cpp | Set pin mode relay sebagai OUTPUT, kontaktor sebagai INPUT_PULLUP |
| `setRelay(idx, state)` | relay_handler.cpp | Set relay ON/OFF (dengan proteksi index out of range) |
| `readKontaktor()` | relay_handler.cpp | Baca 4 input kontaktor dengan software debounce 50ms |
| `initModbus()` | modbus_handler.cpp | Inisialisasi Serial2 dan pin DE/RE MAX485 |
| `readPZEM(addr, &data)` | modbus_handler.cpp | Baca register tegangan, arus, daya, energi via Modbus |
| `initMQTT()` | mqtt_handler.cpp | Konek WiFi, set MQTT server, subscribe topik |
| `mqttLoop()` | mqtt_handler.cpp | Maintain koneksi, publish data periodik, heartbeat |
| `callback(topic, payload, len)` | mqtt_handler.cpp | Handle perintah MQTT masuk (relay + konfigurasi) |
| `processSerial()` | serial_handler.cpp | Parse dan eksekusi perintah dari Serial Monitor |
| `publishData()` | mqtt_handler.cpp | Kirim data PZEM ke `plc/data` (JSON) |
| `publishHeartbeat()` | mqtt_handler.cpp | Kirim status device ke `plc/heartbeat` (JSON) |

---

## Pengembangan

Berikut ide pengembangan yang bisa ditambahkan:

### Fitur Tambahan

| Fitur | Deskripsi |
|-------|-----------|
| **OTA Update** | Update firmware via WiFi tanpa kabel USB |
| **Deep Sleep** | Hemat daya dengan mode sleep saat tidak ada beban |
| **SD Card Logging** | Simpan data ke microSD untuk historical data |
| **Email / Telegram Alert** | Notifikasi jika nilai melebihi threshold |
| **Web Dashboard** | Interface web langsung dari ESP32 |
| **Modbus TCP** | Selain Modbus RTU, dukung Modbus TCP via Ethernet |
| **Multiple PZEM** | Dukung >2 PZEM-017 di bus yang sama |
| **Kalibrasi** | Set offset untuk koreksi pembacaan |

### Skenario Lanjutan

- **Integrasi dengan Home Assistant** — Data PZEM bisa dibaca via MQTT di Home Assistant
- **Automasi** — Relay otomatis ON/OFF berdasarkan threshold arus/tegangan
- **Data Logger** — Simpan data ke Google Sheets via MQTT → Node-RED
- **Multi Device** — Multiple PLC terhubung ke 1 broker untuk monitoring panel distribusi

---

## Lisensi

Proyek ini bersifat open source. Silakan gunakan, modifikasi, dan distribusikan sesuai kebutuhan.

---

> **Power Line Conditioner v1.0** — Dibuat dengan ESP32, PZEM-017, Modbus RTU, dan MQTT.
