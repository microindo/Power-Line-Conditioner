#include "modbus_handler.h"

ModbusMaster node;
PZEMData pzemInput = {0, 0, 0, 0, 0, 0, false};
PZEMData pzemOutput = {0, 0, 0, 0, 0, 0, false};

void initModbus() {
  Serial2.begin(PZEM_BAUD, SERIAL_8N1, PIN_RS485_RO, PIN_RS485_DI);
}

void readPZEM(uint8_t addr, PZEMData &data) {
  node.begin(addr, Serial2);
  uint8_t result;

  result = node.readInputRegisters(REG_VOLTAGE, 7);
  if (result == node.ku8MBSuccess) {
    uint16_t rawVoltage = node.getResponseBuffer(0);
    uint16_t rawCurrent = node.getResponseBuffer(1);
    uint16_t rawPower   = node.getResponseBuffer(2);
    uint16_t rawEnergyH = node.getResponseBuffer(3);
    uint16_t rawEnergyL = node.getResponseBuffer(4);
    uint16_t rawFreq    = node.getResponseBuffer(5);
    uint16_t rawPF      = node.getResponseBuffer(6);

    data.voltage      = rawVoltage / 10.0f;
    data.current      = rawCurrent / 1000.0f;
    data.power        = rawPower / 10.0f;
    data.energy       = (uint32_t)rawEnergyH << 16 | rawEnergyL;
    data.frequency    = rawFreq / 10.0f;
    data.powerFactor  = rawPF / 100.0f;
    data.valid        = true;
  } else {
    data.valid = false;
  }
}
