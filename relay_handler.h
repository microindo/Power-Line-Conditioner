#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include "config.h"

extern bool relayState[4];
extern bool kontaktorState[4];

void initRelay();
void setRelay(uint8_t index, bool state);
void readKontaktor();
void printRelayStatus();
void printKontaktorStatus();

#endif
