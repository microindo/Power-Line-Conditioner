#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

void initMQTT();
void mqttLoop();
void publishData();
void publishRelayStatus();
void publishKontaktorStatus();
void publishHeartbeat();
bool mqttConnected();

#endif
