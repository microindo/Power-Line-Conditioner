#ifndef MODBUS_H
#define MODBUS_H

#include <ModbusMaster.h>
#include "config.h"

extern PZEMData pzemInput;
extern PZEMData pzemOutput;

void initModbus();
void readPZEM(uint8_t addr, PZEMData &data);

#endif
