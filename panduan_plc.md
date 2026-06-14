---
title: |
  \textbf{\huge Power Line Conditioner}\\
  \large Panduan Instalasi, Konfigurasi \& Uji Coba
subtitle: ESP32 + PZEM-016 + MQTT
author: microindo
date: Juni 2026
toc: true
toc-depth: 3
numbersections: true
documentclass: report
classoption: [a4paper, 12pt]
geometry: margin=2.5cm
header-includes:
  - \usepackage{fancyhdr}
  - \pagestyle{fancy}
  - \fancyhead[L]{\leftmark}
  - \fancyhead[R]{Power Line Conditioner}
  - \fancyfoot[C]{\thepage}
  - \usepackage{graphicx}
  - \usepackage{hyperref}
  - \hypersetup{colorlinks=true,linkcolor=blue,urlcolor=blue}
  - \usepackage{xcolor}
  - \usepackage{listings}
  - \lstset{basicstyle=\footnotesize\ttfamily,breaklines=true,frame=single,backgroundcolor=\color{gray!10}}
---

\newpage

# Pendahuluan

## Gambaran Umum

**Power Line Conditioner** adalah sistem monitoring dan kontrol power line berbasis **ESP32** yang dilengkapi dengan:

- 2 buah sensor **PZEM-016** (Modbus RS485) untuk pengukuran tegangan, arus, daya, dan energi di sisi input dan output.
- 4 channel **relay output** untuk kontrol beban.
- 4 input **kontaktor** untuk membaca status ON/OFF.
- Komunikasi **MQTT** untuk integrasi IoT cloud.
- Konfigurasi penuh melalui **Serial Monitor** dan **MQTT**.
- **Watchdog timer** untuk menjamin keandalan sistem.
- **Heartbeat** periodik untuk monitoring status device.

## Tujuan Dokumen

Dokumen ini bertujuan sebagai panduan lengkap mulai dari:

1. Perakitan hardware dan wiring
2. Instalasi software dan library
3. Konfigurasi awal sistem
4. Penggunaan perintah serial dan MQTT
5. Prosedur uji coba sistematis
6. Troubleshooting masalah umum
7. Fitur RESET FACTORY

## Spesifikasi Teknis

| Parameter | Nilai |
|-----------|-------|
| Mikrokontroler | ESP32 (Xtensa LX6 dual-core) |
| Tegangan Operasi | 5V DC |
| Komunikasi Sensor | Modbus RTU RS485 (9600 baud) |
| Komunikasi IoT | MQTT via WiFi (2.4 GHz) |
| Relay Output | 4 channel, Active LOW |
| Input Kontaktor | 4 channel, pull-up internal |
| Interval Data | 100 -- 60000 ms (konfigurabel) |
| Heartbeat | 5 menit |
| Watchdog | 10 detik |
| Penyimpanan | NVS Preferences (non-volatile) |

\newpage

# Perangkat Keras

## Daftar Komponen

| No | Komponen | Spesifikasi | Jumlah |
|----|----------|-------------|--------|
| 1 | ESP32 Board | DOIT ESP32 DEVKIT V1 | 1 |
| 2 | PZEM-016 | DC Energy Meter Modbus | 2 |
| 3 | MAX485 | RS485 to TTL (auto direction) | 1 |
| 4 | Relay Module | 4-Channel 5V Active LOW | 1 |
| 5 | Power Supply 5V | 5V / 2A untuk ESP32 | 1 |
| 6 | Push Button | Tactile switch | 4 |
| 7 | Resistor | 10 k$\Omega$ | 4 |
| 8 | Kabel Jumper | Male-to-Male, Male-to-Female | 30 |
| 9 | Breadboard | 830 point | 1 |
| 10 | Kabel Micro USB | Data + Power | 1 |

## Diagram Blok

```
                    +------------------+
                    |   Power Supply   |
                    |    (5V / 12V)    |
                    +--------+---------+
                             |
                    +--------+---------+
                    |      ESP32        |
                    |                   |
                    | GPIO16 --- RXD --+---- MAX485 --+-- PZEM Input
                    | GPIO17 --- TXD --+              |-- PZEM Output
                    |                   |             |
                    | GPIO26 --- Relay 1              |
                    | GPIO27 --- Relay 2              |
                    | GPIO32 --- Relay 3              |
                    | GPIO33 --- Relay 4              |
                    |                   |             |
                    | GPIO34 --- Kontaktor 1          |
                    | GPIO35 --- Kontaktor 2          |
                    | GPIO36 --- Kontaktor 3          |
                    | GPIO39 --- Kontaktor 4          |
                    |                   |             |
                    | GPIO2  --- LED Indikator        |
                    +--------+---------+
```

## Wiring Detail

### MAX485 (Auto Direction) ke ESP32

| MAX485 | ESP32 | Kabel |
|--------|-------|-------|
| RXD | GPIO16 (UART2 RX) | Hijau |
| TXD | GPIO17 (UART2 TX) | Putih |
| VCC | 5V | Merah |
| GND | GND | Hitam |
| A (RS485+) | PZEM-016 (A) | Biru |
| B (RS485-) | PZEM-016 (B) | Kuning |

CATATAN: Modul MAX485 auto direction hanya memiliki 4 pin TTL (VCC, GND, TXD, RXD). Tidak perlu pin kontrol DE/RE.

### Relay Module ke ESP32

| Relay Module | ESP32 |
|-------------|-------|
| IN1 | GPIO26 |
| IN2 | GPIO27 |
| IN3 | GPIO32 |
| IN4 | GPIO33 |
| VCC | 5V |
| GND | GND |

### Kontaktor (Push Button) ke ESP32

| Push Button | ESP32 |
|-------------|-------|
| Kaki 1 | GPIO yang sesuai (34/35/36/39) |
| Kaki 1 | 10 k$\Omega$ ke GND (pull-down) |
| Kaki 2 | 3.3V |

### PZEM-016 ke Bus RS485

Kedua PZEM-016 terhubung secara paralel ke bus RS485 yang sama:

| PZEM-016 | MAX485 |
|----------|--------|
| A (+) | A (+) |
| B (-) | B (-) |
| Power | 12V DC (sumber eksternal) |

Perbedaan alamat Modbus (0x01 dan 0x02) yang membedakan kedua unit.

\newpage

# Instalasi Software

## Instalasi Arduino IDE

1. Download Arduino IDE dari \url{https://www.arduino.cc/en/software}
2. Install sesuai sistem operasi
3. Buka Arduino IDE

## Instalasi Board ESP32

1. Buka **File > Preferences**
2. Pada **Additional Boards Manager URLs**, tambahkan:

   \url{https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json}

3. Klik **OK**
4. Buka **Tools > Board > Boards Manager**
5. Cari **"ESP32"** lalu klik **Install**

## Instalasi Library

Buka **Sketch > Include Library > Manage Libraries**.

Cari dan install library berikut:

| Library | Pencarian | Versi Min |
|---------|-----------|-----------|
| ModbusMaster | `modbusmaster` | 2.0 |
| PubSubClient | `pubsubclient` | 2.8 |
| ArduinoJson | `arduinojson` | 6.18 |

## Download Source Code

Clone repositori:

```bash
git clone https://github.com/microindo/Power-Line-Conditioner.git
```

Atau download ZIP dari halaman GitHub (Code > Download ZIP), ekstrak, lalu buka \texttt{Power\_Line\_Conditioner.ino} di Arduino IDE.

\newpage

# Konfigurasi PZEM-016

## Mengubah Alamat Modbus

PZEM-016 memiliki alamat default \texttt{0x01}. Untuk menggunakan 2 unit, alamat salah satu harus diubah menjadi \texttt{0x02}.

### Metode: Menggunakan ESP32

Upload kode berikut satu kali untuk mengubah alamat:

```cpp
#include <ModbusMaster.h>

ModbusMaster node;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Ubah alamat dari 0x01 ke 0x02
  node.begin(0x01, Serial2);
  uint16_t addr[1] = {0x0002};
  node.writeSingleRegister(0x0002, addr[0]);

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

CATATAN:

- Lepas PZEM-016 kedua dari bus saat melakukan perubahan
- Beri label fisik pada modul yang sudah diubah
- Setelah selesai, upload ulang program utama

### Konfigurasi di Code

Di \texttt{config.h}:

```cpp
#define PZEM_ADDR_INPUT   0x01   // PZEM sisi input
#define PZEM_ADDR_OUTPUT  0x02   // PZEM sisi output
```

\newpage

# Upload Program

## Langkah Upload

1. Buka \texttt{Power\_Line\_Conditioner.ino} di Arduino IDE
2. Pilih board: **Tools > Board > ESP32 Dev Module**
3. Pilih port: **Tools > Port >** (sesuai dengan port ESP32)
4. Klik **Upload** ($\rightarrow$)
5. Jika gagal, tahan tombol **BOOT** saat upload dimulai

## Verifikasi Upload

Setelah upload berhasil, buka **Serial Monitor** (baud 115200). Akan tampil:

```
=== Power Line Conditioner v1.0 ===
Ketik HELP untuk daftar perintah
System ready.
```

Jika tidak muncul, tekan tombol **EN/RST** pada ESP32.

\newpage

# Konfigurasi Awal

Semua konfigurasi dilakukan melalui **Serial Monitor** (Tools > Serial Monitor, baud: 115200).

## Langkah 1: Set WiFi

```
SET SSID NamaWiFi_Anda
SET PASSWORD PasswordWiFi_Anda
SAVE
RESTART
```

ESP32 akan restart dan mencoba konek ke WiFi.

## Langkah 2: Set MQTT Broker

```
SET BROKER broker.hivemq.cloud
SET PORT 1883
SET BROKER_USER username_anda
SET BROKER_PASS password_anda
SAVE
RESTART
```

## Langkah 3: Verifikasi

Setelah restart, ketik \texttt{STATUS}:

```
--- Konfigurasi ---
SSID: NamaWiFi_Anda
Password: ***
Broker: broker.hivemq.cloud
Port: 1883
...

WiFi: Terhubung
MQTT: Terhubung
...
```

\newpage

# Daftar Perintah Serial

## Perintah Konfigurasi

| Perintah | Contoh | Deskripsi |
|----------|--------|-----------|
| `SET SSID <nama>` | `SET SSID Rumah_Saya` | Set nama WiFi |
| `SET PASSWORD <pass>` | `SET PASSWORD rahasia` | Set password WiFi |
| `SET BROKER <host>` | `SET BROKER test.mosquitto.org` | Set host MQTT broker |
| `SET PORT <port>` | `SET PORT 1883` | Set port MQTT |
| `SET BROKER_USER <user>` | `SET BROKER_USER admin` | Set username MQTT |
| `SET BROKER_PASS <pass>` | `SET BROKER_PASS admin123` | Set password MQTT |
| `SET DELAY <ms>` | `SET DELAY 5000` | Set interval kirim data (100-60000 ms) |

## Perintah Kontrol

| Perintah | Contoh | Deskripsi |
|----------|--------|-----------|
| `SET RELAY <1-4> <0/1>` | `SET RELAY 1 1` | Relay 1 ON (1) / OFF (0) |

## Perintah Sistem

| Perintah | Deskripsi |
|----------|-----------|
| `HELP` | Tampilkan daftar perintah |
| `STATUS` | Tampilkan semua konfigurasi, status, dan sensor |
| `SAVE` | Simpan konfigurasi ke NVS (tahan restart) |
| `RESET` | Reset konfigurasi ke default |
| `RESET FACTORY` | Factory reset + matikan relay + restart |
| `RESTART` | Restart ESP32 |

## Contoh Sesi

```
> SET RELAY 1 1
Relay 1 ON

> SET RELAY 2 1
Relay 2 ON

> STATUS
--- Konfigurasi ---
SSID: Rumah_Saya
...
Relay: R1=ON R2=ON R3=OFF R4=OFF
Kontaktor: C1=ON C2=OFF C3=OFF C4=OFF
PZEM Input: 24.10V 0.50A 12.05Wh
PZEM Output: 24.08V 0.48A 11.56Wh
WiFi: Terhubung
MQTT: Terhubung
Uptime: 360 detik

> SAVE
Konfigurasi tersimpan
```

\newpage

# MQTT Topics

## Topik Publish (ESP32 $\rightarrow$ Cloud)

### \texttt{plc/data}

Data PZEM input dan output (JSON):

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

### \texttt{plc/status/relay}

```json
{"r1":"ON","r2":"OFF","r3":"ON","r4":"OFF"}
```

### \texttt{plc/status/contactor}

```json
{"c1":"ON","c2":"OFF","c3":"ON","c4":"OFF"}
```

### \texttt{plc/heartbeat}

Dikirim setiap 5 menit:

```json
{"uptime":600,"rssi":-65,"heap":198000,"ver":"1.0"}
```

### \texttt{plc/online}

Last Will \& Testament: \texttt{online} saat konek, \texttt{offline} saat putus.

## Topik Subscribe (Cloud $\rightarrow$ ESP32)

### Kontrol Relay

| Topik | Payload | Efek |
|-------|---------|------|
| `plc/relay/1/set` | `ON` / `OFF` | Relay 1 ON/OFF |
| `plc/relay/2/set` | `ON` / `OFF` | Relay 2 ON/OFF |
| `plc/relay/3/set` | `ON` / `OFF` | Relay 3 ON/OFF |
| `plc/relay/4/set` | `ON` / `OFF` | Relay 4 ON/OFF |

### Konfigurasi via MQTT

| Topik | Payload | Efek |
|-------|---------|------|
| `plc/config/wifi_ssid` | string | Set WiFi SSID (auto save) |
| `plc/config/wifi_pass` | string | Set WiFi password |
| `plc/config/mqtt_host` | string | Set MQTT broker host |
| `plc/config/mqtt_port` | number | Set MQTT port |
| `plc/config/mqtt_user` | string | Set username MQTT |
| `plc/config/mqtt_pass` | string | Set password MQTT |
| `plc/config/send_delay` | number | Set delay kirim (ms) |

Perubahan konfigurasi via MQTT akan tersimpan otomatis ke NVS.

\newpage

# Panduan Uji Coba

## Tahap 1: Power-On Test

**Tujuan:** Memastikan ESP32 menyala dan program berjalan.

1. Hubungkan ESP32 ke power supply / USB
2. Buka Serial Monitor (baud 115200)
3. Cek pesan:
   ```
   === Power Line Conditioner v1.0 ===
   System ready.
   ```
4. Jika tidak muncul, tekan tombol EN/RST
5. Ketik `HELP` pastikan daftar perintah tampil

**Kriteria Sukses:** Serial Monitor menampilkan pesan dan merespon perintah.

## Tahap 2: Test Serial Commands

**Tujuan:** Verifikasi semua perintah serial.

| Langkah | Perintah | Hasil |
|---------|----------|-------|
| 1 | `STATUS` | Tampil semua konfigurasi dan status |
| 2 | `SET RELAY 1 1` | Relay 1 ON, relay di pin 26 menyala |
| 3 | `SET RELAY 1 0` | Relay 1 OFF, relay mati |
| 4 | `SET DELAY 10000` | Delay diubah ke 10000 ms |
| 5 | `SET DELAY 50` | Error: Delay 100-60000 ms (validasi OK) |
| 6 | `SAVE` | Konfigurasi tersimpan |
| 7 | `RESTART` | ESP32 restart, delay 10000 masih ada |

**Kriteria Sukses:** Semua perintah merespon sesuai.

## Tahap 3: Test PZEM-016

**Tujuan:** Verifikasi komunikasi Modbus dengan kedua PZEM-016.

**Prasyarat:** MAX485 terhubung ke kedua PZEM-016, PZEM mendapat power 12V.

1. Buka Serial Monitor
2. Ketik `STATUS`
3. Cek baris:
   ```
   PZEM Input: 24.12V 0.50A 12.06W 0.05Wh
   PZEM Output: 24.08V 0.48A 11.56W 0.03Wh
   ```

**Jika gagal (tidak terhubung):**
- Periksa kabel A/B (jika terbalik, tukar)
- Periksa alamat Modbus (harus 0x01 dan 0x02)
- Pastikan power PZEM-016 aktif (12V)
- Pastikan MAX485 terhubung ke GPIO16 (RXD) dan GPIO17 (TXD)

**Kriteria Sukses:** Kedua PZEM menampilkan data yang valid.

## Tahap 4: Test Relay

**Tujuan:** Verifikasi 4 relay dapat dikontrol.

**Alat:** Multimeter atau LED + resistor 220$\Omega$.

1. Hubungkan multimeter ke COM dan NO relay 1
2. `SET RELAY 1 1` $\rightarrow$ multimeter menunjukkan kontak menutup (0$\Omega$)
3. `SET RELAY 1 0` $\rightarrow$ kontak terbuka (OL)
4. Ulangi untuk relay 2, 3, 4

**Test simultan:**
```
SET RELAY 1 1
SET RELAY 2 1
SET RELAY 3 1
SET RELAY 4 1
STATUS
```
Output: `Relay: R1=ON R2=ON R3=ON R4=ON`

**Kriteria Sukses:** Semua relay ON/OFF sesuai perintah.

## Tahap 5: Test Input Kontaktor

**Tujuan:** Verifikasi 4 input kontaktor.

**Alat:** 4 push button atau kabel jumper.

1. Hubungkan GPIO34 ke 3.3V $\rightarrow$ `STATUS` $\rightarrow$ `C1=ON`
2. Lepas GPIO34 $\rightarrow$ `STATUS` $\rightarrow$ `C1=OFF`
3. Ulangi untuk GPIO35, GPIO36, GPIO39

**Test bouncing:** Tekan dan lepas button cepat berkali-kali. Debouncing 50ms mencegah pembacaan ganda.

**Kriteria Sukses:** Status kontaktor berubah sesuai input.

## Tahap 6: Test WiFi

**Tujuan:** Verifikasi koneksi WiFi.

1. Konfigurasi SSID dan password (lihat Bab Konfigurasi Awal)
2. Setelah restart, Serial Monitor menampilkan:
   ```
   Menghubungkan WiFi....
   WiFi terhubung
   IP: 192.168.1.100
   ```
3. `STATUS` $\rightarrow$ `WiFi: Terhubung`

**Kriteria Sukses:** ESP32 mendapat IP dan WiFi terhubung.

## Tahap 7: Test MQTT

**Tujuan:** Verifikasi komunikasi MQTT dua arah.

**Prasyarat:** WiFi sudah terhubung.

**Alat:** MQTT client (MQTTX, mosquitto, atau tool lain).

### Test Subscribe (Kontrol dari Cloud)
1. Subscribe ke semua topik:
   ```
   mosquitto_sub -h broker.hivemq.cloud -t "plc/#" -v
   ```
2. Kirim perintah:
   ```
   mosquitto_pub -h broker.hivemq.cloud -t "plc/relay/1/set" -m "ON"
   ```
3. ESP32 merespon:
   ```
   plc/status/relay {"r1":"ON","r2":"OFF","r3":"OFF","r4":"OFF"}
   ```

### Test Publish (ESP32 ke Cloud)
Setelah beberapa detik akan muncul data:
```
plc/data {"v_in":24.12,"i_in":0.50,...}
plc/status/relay {"r1":"ON",...}
plc/status/contactor {"c1":"ON",...}
```

### Test Konfigurasi via MQTT
Kirim perintah konfigurasi lewat MQTT:
```
mosquitto_pub -h broker.hivemq.cloud -t "plc/config/send_delay" -m "3000"
```
ESP32 akan menyimpan dan menggunakan delay 3 detik.

### Test LWT
Matikan power ESP32. Dalam beberapa detik akan muncul:
```
plc/online offline
```

**Kriteria Sukses:** Semua topik publish/subscribe berfungsi.

## Tahap 8: Test Heartbeat

**Tujuan:** Verifikasi heartbeat 5 menit.

1. Catat waktu start ESP32
2. Tunggu 5 menit
3. Di MQTT client muncul:
   ```
   plc/heartbeat {"uptime":300,"rssi":-65,"heap":195000,"ver":"1.0"}
   ```
4. Tunggu 5 menit lagi:
   ```
   plc/heartbeat {"uptime":600,"rssi":-64,"heap":193000,"ver":"1.0"}
   ```

**Kriteria Sukses:** Heartbeat muncul setiap 300 detik.

## Tahap 9: Test RESET FACTORY

**Tujuan:** Verifikasi fitur factory reset.

**Peringatan:** Perintah ini akan menghapus semua konfigurasi dan merestart ESP32.

1. Pastikan sudah ada konfigurasi tersimpan (WiFi, MQTT, dll)
2. Test relay dalam kondisi ON:
   ```
   SET RELAY 1 1
   SET RELAY 2 1
   ```
3. Ketik perintah factory reset:
   ```
   RESET FACTORY
   ```
4. ESP32 akan menampilkan:
   ```
   FACTORY RESET...
   Konfigurasi direset ke default
   Factory reset selesai. Restart...
   ```
5. ESP32 restart otomatis
6. Setelah restart, cek dengan `STATUS`:
   - Semua konfigurasi kembali ke default (SSID kosong, broker kosong, delay=5000 ms)
   - Semua relay dalam kondisi OFF
7. Cek MQTT client: ESP32 sudah offline (konfigurasi WiFi hilang)

**Kriteria Sukses:**
- Semua konfigurasi kembali ke nilai default
- Semua relay mati
- ESP32 restart otomatis

## Tahap 10: Test Watchdog

**Tujuan:** Verifikasi watchdog bekerja.

1. Tambahkan baris berikut di `loop()` untuk simulasi hang:
   ```cpp
   while (true) {}  // infinite loop
   ```
2. Upload dan tunggu 10 detik
3. ESP32 akan restart otomatis (terlihat di Serial Monitor)
4. Hapus baris tersebut dan upload ulang program normal

**Kriteria Sukses:** ESP32 restart otomatis saat hang tanpa intervensi manual.

## Tahap 11: Test End-to-End

**Tujuan:** Simulasi skenario dunia nyata.

### Skenario 1: Monitor Jarak Jauh
1. Buka MQTT client, subscribe ke `plc/#`
2. Amati data tegangan, arus, daya muncul setiap `send_delay`
3. Pastikan `input_valid` dan `output_valid` = true

### Skenario 2: Kontrol Jarak Jauh
1. Kirim `plc/relay/1/set` $\rightarrow$ `ON`
2. Beban menyala, status relay berubah
3. PZEM output mulai membaca arus
4. Kirim `plc/relay/1/set` $\rightarrow$ `OFF`
5. Beban mati, arus PZEM output = 0

### Skenario 3: Konfigurasi Ulang Jarak Jauh
1. Ubah delay via MQTT: `plc/config/send_delay` $\rightarrow$ `2000`
2. Data terkirim setiap 2 detik
3. Matikan power ESP32, nyalakan lagi
4. Data tetap terkirim setiap 2 detik (konfigurasi tersimpan di NVS)

**Kriteria Sukses:** Semua skenario berjalan sesuai.

\newpage

# Troubleshooting

## Masalah Hardware

| Masalah | Kemungkinan Penyebab | Solusi |
|---------|----------------------|--------|
| ESP32 tidak menyala | Power tidak cukup | Gunakan 5V/2A |
| Serial Monitor kosong | Port salah | Cek port di Tools > Port |
| | Board salah | Pilih ESP32 Dev Module |
| PZEM data tidak terbaca | Kabel A/B terbalik | Tukar kabel A dan B |
| | Alamat salah | Cek alamat Modbus |
| | Power PZEM mati | PZEM butuh 12V DC |
| Relay tidak menyala | Power relay kurang | Supply terpisah untuk relay |
| Kontaktor tidak terbaca | Pull-down salah | Pasang resistor 10k$\Omega$ |

## Masalah Software

| Masalah | Solusi |
|---------|--------|
| Upload gagal | Tahan tombol BOOT saat upload |
| ESP32 loop restart | Cek Serial Monitor untuk stack trace |
| WiFi tidak konek | Cek SSID/password, pastikan AP aktif |
| MQTT tidak konek | Cek host/port/user/pass, broker aktif |
| Konfigurasi hilang | Jalankan `SAVE` sebelum `RESTART` |

## Cara Debugging

1. **Serial Monitor:** `STATUS` untuk lihat semua parameter
2. **MQTT:** Subscribe ke `plc/#` untuk semua traffic
3. **PZEM:** Cek field `input_valid`/`output_valid`
4. **Heap:** Jika terus menurun, ada memory leak
5. **Watchdog:** Jika restart sendiri, ada code yang hang

\newpage

# Fitur RESET FACTORY

## Deskripsi

Fitur RESET FACTORY mengembalikan seluruh sistem ke kondisi awal pabrik. Berguna ketika:

- Ingin mengkonfigurasi ulang dari awal
- Lupa kredensial WiFi/MQTT
- Akan memindahkan device ke lokasi/jaringan baru
- Mengalami masalah konfigurasi yang tidak bisa diperbaiki

## Cara Penggunaan

### Via Serial Monitor

```
RESET FACTORY
```

Output:
```
FACTORY RESET...
Konfigurasi direset ke default
Factory reset selesai. Restart...
```

### Via MQTT

Kirim perintah ke topik konfigurasi (dapat dikembangkan sesuai kebutuhan).

## Yang Dilakukan RESET FACTORY

1. **Hapus semua data NVS** -- SSID, password WiFi, host/port/user/pass MQTT, delay, semuanya dihapus
2. **Kembalikan nilai default** -- `send_delay` kembali ke 5000 ms, `mqtt_port` ke 1883
3. **Matikan semua relay** -- Relay 1-4 di OFF kan
4. **Restart ESP32** -- System restart agar menerapkan konfigurasi baru

## Yang Tidak Dilakukan

- Tidak mengubah firmware (tidak perlu upload ulang)
- Tidak mengubah alamat PZEM-016 (tetap 0x01 dan 0x02)
- Tidak mengubah pin assignment (hardware tetap sama)

## Perbandingan RESET vs RESET FACTORY

| Aspek | RESET | RESET FACTORY |
|-------|-------|---------------|
| Hapus NVS config | Ya | Ya |
| Matikan relay | Tidak | Ya |
| Restart otomatis | Tidak | Ya |
| Penggunaan | Reset config saja | Factory reset total |

\newpage

# Lampiran

## A. Struktur File Proyek

```
Power Line Conditioner/
├── Power_Line_Conditioner.ino   # Main program
├── config.h                     # Pin definitions, konstanta
├── storage.h / storage.cpp      # Baca/tulis NVS Preferences
├── relay_handler.h / .cpp       # Kontrol relay & kontaktor
├── modbus_handler.h / .cpp      # Komunikasi Modbus PZEM-016
├── mqtt_handler.h / .cpp        # MQTT client
├── serial_handler.h / .cpp      # Serial command parser
├── README.md                    # Dokumentasi
└── .gitignore
```

## B. Library yang Dibutuhkan

| Library | Fungsi |
|---------|--------|
| ModbusMaster | Komunikasi Modbus RTU via RS485 |
| PubSubClient | MQTT client untuk ESP32 |
| ArduinoJson | Membuat dan parsing JSON |
| WiFi (built-in) | Koneksi WiFi |
| Preferences (built-in) | Penyimpanan NVS |

## C. Referensi

- Dokumentasi ESP32 Arduino: \url{https://docs.espressif.com/projects/arduino-esp32/}
- Dokumentasi Blynk: \url{https://docs.blynk.io/}
- Modbus Protocol: \url{https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf}
- PZEM-016 Datasheet: \url{https://www.youtu.be/peacefair-pzem-016}

## D. Riwayat Versi

| Versi | Tanggal | Keterangan |
|-------|---------|------------|
| 1.0 | Juni 2026 | Rilis awal |
