#include "storage.h"

Preferences prefs;
AppConfig appConfig;

void loadConfig() {
  prefs.begin("plc", false);

  {
    String tmp;
    tmp = prefs.getString("wifi_ssid", "");
    strcpy(appConfig.wifi_ssid, tmp.c_str());
    tmp = prefs.getString("wifi_pass", "");
    strcpy(appConfig.wifi_pass, tmp.c_str());
    tmp = prefs.getString("mqtt_host", "");
    strcpy(appConfig.mqtt_host, tmp.c_str());
    tmp = prefs.getString("mqtt_user", "");
    strcpy(appConfig.mqtt_user, tmp.c_str());
    tmp = prefs.getString("mqtt_pass", "");
    strcpy(appConfig.mqtt_pass, tmp.c_str());
  }
  appConfig.mqtt_port = prefs.getUShort("mqtt_port", DEFAULT_MQTT_PORT);
  appConfig.send_delay = prefs.getUInt("send_delay", DEFAULT_SEND_DELAY);

  prefs.end();
}

void saveConfig() {
  prefs.begin("plc", false);
  prefs.putString("wifi_ssid", appConfig.wifi_ssid);
  prefs.putString("wifi_pass", appConfig.wifi_pass);
  prefs.putString("mqtt_host", appConfig.mqtt_host);
  prefs.putUShort("mqtt_port", appConfig.mqtt_port);
  prefs.putString("mqtt_user", appConfig.mqtt_user);
  prefs.putString("mqtt_pass", appConfig.mqtt_pass);
  prefs.putUInt("send_delay", appConfig.send_delay);
  prefs.end();
}

void resetConfig() {
  prefs.begin("plc", false);
  prefs.clear();
  prefs.end();
  loadConfig();
}

void printConfig() {
  Serial.println("--- Konfigurasi ---");
  Serial.print("SSID: "); Serial.println(appConfig.wifi_ssid);
  Serial.print("Password: "); Serial.println(strlen(appConfig.wifi_pass) > 0 ? "***" : "(kosong)");
  Serial.print("Broker: "); Serial.println(appConfig.mqtt_host);
  Serial.print("Port: "); Serial.println(appConfig.mqtt_port);
  Serial.print("User: "); Serial.println(appConfig.mqtt_user);
  Serial.print("Pass: "); Serial.println(strlen(appConfig.mqtt_pass) > 0 ? "***" : "(kosong)");
  Serial.print("Send Delay: "); Serial.print(appConfig.send_delay); Serial.println(" ms");
  Serial.println("-------------------");
}
