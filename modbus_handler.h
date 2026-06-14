#ifndef MODBUS_H
#define MODBUS_H

#include <ModbusMaster.h>
#include "config.h"

extern PZEMData pzemInput;
extern PZEMData pzemOutput;

void initModbus();
void readPZEM(uint8_t addr, PZEMData &data);
void setPZEMAddress(uint8_t oldAddr, uint8_t newAddr);

#endif
