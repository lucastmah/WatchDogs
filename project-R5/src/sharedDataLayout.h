#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

// R5 Shared Memory Note
// - It seems that using a struct for the ATCM memory does not work 
//   (hangs when accessing memory via a struct pointer).
// - Therefore, using an array.

#define MSG_OFFSET 0x7000
#define LED_STRIP_OFFSET_0 MSG_OFFSET
#define LED_STRIP_OFFSET_1 (LED_STRIP_OFFSET_0 + sizeof(uint32_t))
#define LED_STRIP_OFFSET_2 (LED_STRIP_OFFSET_1 + sizeof(uint32_t))
#define LED_STRIP_OFFSET_3 (LED_STRIP_OFFSET_2 + sizeof(uint32_t))
#define LED_STRIP_OFFSET_4 (LED_STRIP_OFFSET_3 + sizeof(uint32_t))
#define LED_STRIP_OFFSET_5 (LED_STRIP_OFFSET_4 + sizeof(uint32_t))
#define LED_STRIP_OFFSET_6 (LED_STRIP_OFFSET_5 + sizeof(uint32_t))
#define LED_STRIP_OFFSET_7 (LED_STRIP_OFFSET_6 + sizeof(uint32_t))

// #define MEM_UINT8(addr) (uint8_t*)(addr)
#define MEM_UINT32(addr) *(uint32_t*)(addr)

#endif
