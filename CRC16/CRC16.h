#ifndef __CRC_16_H
#define __CRC_16_H

#include "ch32v30x.h"

u16 compute_crc16(u8* addr, u32 num);     // Look-up table calculation method

#endif
