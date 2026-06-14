#include <ArduinoJson.h>
#include "mqtt_handler.h"
#include "relay_handler.h"
#include "modbus_handler.h"

WiFiClient espClient;
PubSubClient client(espClient);

static unsigned long lastSend = 0;
static unsigned long lastHeartbeat = 0;
static unsigned long startTime = 0;

bool mqttConnected() {
  return client.connected();
}

static void callback(char* topic, byte* payload, unsigned int length);
static void connectMQTT();
static void connectWiFi();

void initMQTT() {
  startTime = millis();
  client.setServer(appConfig.mqtt_host, appConfig.mqtt_port);
  client.setCallback(callback);
  connectWiFi();
}

void mqttLoop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  unsigned long now = millis();

  if (now - lastSend >= appConfig.send_delay) {
    lastSend = now;
    publishData();
    publishRelayStatus();
    publishKontaktorStatus();
  }

  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = now;
    publishHeartbeat();
  }
}

bool mqttConnected() {
  return client.connected();
}

static void connectWiFi() {
  if (strlen(appConfig.wifi_ssid) == 0) return;
  Serial.print("Menghubungkan WiFi");
  WiFi.begin(appConfig.wifi_ssid, appConfig.wifi_pass);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi terhubung");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi gagal");
  }
}

static void connectMQTT() {
  if (strlen(appConfig.mqtt_host) == 0) return;
  Serial.print("Menghubungkan MQTT");
  String clientId = "PLC-" + String((uint32_t)ESP.getEfuseMac());
  if (strlen(appConfig.mqtt_user) > 0) {
    client.connect(clientId.c_str(), appConfig.mqtt_user, appConfig.mqtt_pass, "plc/online", 0, true, "offline");
  } else {
    client.connect(clientId.c_str());
  }
  if (client.connected()) {
    Serial.println("OK");
    client.publish("plc/online", "online", true);
    client.subscribe("plc/relay/1/set");
    client.subscribe("plc/relay/2/set");
    client.subscribe("plc/relay/3/set");
    client.subscribe("plc/relay/4/set");
    client.subscribe("plc/config/#");
  } else {
    Serial.print("Gagal: "); Serial.println(client.state());
  }
}

static void callback(char* topic, byte* payload, unsigned int length) {
  char buf[length + 1];
  memcpy(buf, payload, length);
  buf[length] = 0;

  String val = String(buf);
  val.toUpperCase();

  if (strcmp(topic, "plc/relay/1/set") == 0) setRelay(0, val == "ON");
  else if (strcmp(topic, "plc/relay/2/set") == 0) setRelay(1, val == "ON");
  else if (strcmp(topic, "plc/relay/3/set") == 0) setRelay(2, val == "ON");
  else if (strcmp(topic, "plc/relay/4/set") == 0) setRelay(3, val == "ON");
  else if (strcmp(topic, "plc/config/wifi_ssid") == 0) {
    strcpy(appConfig.wifi_ssid, buf);
    saveConfig();
  }
  else if (strcmp(topic, "plc/config/wifi_pass") == 0) {
    strcpy(appConfig.wifi_pass, buf);
    saveConfig();
  }
  else if (strcmp(topic, "plc/config/mqtt_host") == 0) {
    strcpy(appConfig.mqtt_host, buf);
    saveConfig();
  }
  else if (strcmp(topic, "plc/config/mqtt_port") == 0) {
    appConfig.mqtt_port = (uint16_t)val.toInt();
    saveConfig();
  }
  else if (strcmp(topic, "plc/config/mqtt_user") == 0) {
    strcpy(appConfig.mqtt_user, buf);
    saveConfig();
  }
  else if (strcmp(topic, "plc/config/mqtt_pass") == 0) {
    strcpy(appConfig.mqtt_pass, buf);
    saveConfig();
  }
  else if (strcmp(topic, "plc/config/send_delay") == 0) {
    appConfig.send_delay = (uint32_t)val.toInt();
    saveConfig();
  }
}

void publishData() {
  if (!client.connected()) return;

  StaticJsonDocument<256> doc;
  doc["v_in"] = pzemInput.voltage;
  doc["i_in"] = pzemInput.current;
  doc["p_in"] = pzemInput.power;
  doc["e_in"] = pzemInput.energy;
  doc["v_out"] = pzemOutput.voltage;
  doc["i_out"] = pzemOutput.current;
  doc["p_out"] = pzemOutput.power;
  doc["e_out"] = pzemOutput.energy;
  doc["input_valid"] = pzemInput.valid;
  doc["output_valid"] = pzemOutput.valid;

  char buffer[256];
  serializeJson(doc, buffer);
  client.publish("plc/data", buffer);
}

void publishRelayStatus() {
  if (!client.connected()) return;

  char buf[64];
  snprintf(buf, sizeof(buf),
    "{\"r1\":\"%s\",\"r2\":\"%s\",\"r3\":\"%s\",\"r4\":\"%s\"}",
    relayState[0] ? "ON" : "OFF",
    relayState[1] ? "ON" : "OFF",
    relayState[2] ? "ON" : "OFF",
    relayState[3] ? "ON" : "OFF");
  client.publish("plc/status/relay", buf);
}

void publishKontaktorStatus() {
  if (!client.connected()) return;

  char buf[64];
  snprintf(buf, sizeof(buf),
    "{\"c1\":\"%s\",\"c2\":\"%s\",\"c3\":\"%s\",\"c4\":\"%s\"}",
    kontaktorState[0] ? "ON" : "OFF",
    kontaktorState[1] ? "ON" : "OFF",
    kontaktorState[2] ? "ON" : "OFF",
    kontaktorState[3] ? "ON" : "OFF");
  client.publish("plc/status/contactor", buf);
}

void publishHeartbeat() {
  if (!client.connected()) return;

  StaticJsonDocument<128> doc;
  doc["uptime"] = (millis() - startTime) / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["heap"] = ESP.getFreeHeap();
  doc["ver"] = "1.0";

  char buffer[128];
  serializeJson(doc, buffer);
  client.publish("plc/heartbeat", buffer);
}
