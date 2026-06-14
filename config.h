#ifndef CONFIG_H
#define CONFIG_H

// ===== Pin Assignments =====
#define PIN_RS485_RO   16   // UART2 RX ke MAX485 RO
#define PIN_RS485_DI   17   // UART2 TX ke MAX485 DI
// MAX485 auto direction control (4 pin: VCC GND TXD RXD) — tidak perlu DE/RE
#define PIN_RELAY_1    26
#define PIN_RELAY_2    27
#define PIN_RELAY_3    32
#define PIN_RELAY_4    33
#define PIN_KONTAKTOR_1 34
#define PIN_KONTAKTOR_2 35
#define PIN_KONTAKTOR_3 36
#define PIN_KONTAKTOR_4 39
#define PIN_LED        2

// ===== PZEM-017 Addresses =====
#define PZEM_ADDR_INPUT   0x01
#define PZEM_ADDR_OUTPUT  0x02
#define PZEM_BAUD         9600

// ===== Default Config Values =====
#define DEFAULT_SEND_DELAY   5000
#define DEFAULT_MQTT_PORT    1883
#define HEARTBEAT_INTERVAL   300000  // 5 menit
#define WDT_TIMEOUT          10      // detik

// ===== Modbus Register Addresses (PZEM-017) =====
#define REG_VOLTAGE    0x0000
#define REG_CURRENT    0x0001
#define REG_POWER      0x0002
#define REG_ENERGY     0x0003
#define REG_HIGH_ALARM 0x0005
#define REG_LOW_ALARM  0x0006

// ===== PZEM Data Structure =====
struct PZEMData {
  float voltage;
  float current;
  float power;
  float energy;
  bool valid;
};

// ===== Config Structure =====
struct AppConfig {
  char wifi_ssid[32];
  char wifi_pass[64];
  char mqtt_host[64];
  uint16_t mqtt_port;
  char mqtt_user[32];
  char mqtt_pass[32];
  uint32_t send_delay;
};

#endif
