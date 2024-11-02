#pragma once

#include <utils/types.h>

u32 pciConfigReadDWord(u8 bus, u8 slot, u8 function, u8 offset);
u16 pciConfigReadWord(u8 bus, u8 slot, u8 function, u8 offset);
u8 pciConfigReadByte(u8 bus, u8 slot, u8 function, u8 offset);

void pciConfigWrite(u8 bus, u8 slot, u8 function, u8 offset, u32 data);

u16 pciGetVendor(u8 bus, u8 slot);

void pciGetVendors();

void checkBus(u8);
