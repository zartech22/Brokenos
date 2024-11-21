#pragma once

#include <utils/types.h>

uint32_t pciConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint8_t pciConfigReadByte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

void pciConfigWrite(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data);

uint16_t pciGetVendor(uint8_t bus, uint8_t slot);

void pciGetVendors();

void checkBus(uint8_t);
