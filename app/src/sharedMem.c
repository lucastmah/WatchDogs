#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <string.h>

#include "sharedMem.h"
#include "sharedDataLayout.h"

// General R5 Memomry Sharing Routine
// ----------------------------------------------------------------
// #define ATCM_ADDR     0x79000000  // MCU ATCM (p59 TRM)
#define BTCM_ADDR     0x79020000  // MCU BTCM (p59 TRM)
#define MEM_LENGTH    0x8000

// Return the address of the base address of the ATCM memory region for the R5-MCU
volatile void* getR5MmapAddr(void)
{
    // Access /dev/mem to gain access to physical memory (for memory-mapped devices/specialmemory)
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem; Did you run with sudo?");
        exit(EXIT_FAILURE);
    }

    // Inside main memory (fd), access the part at offset BTCM_ADDR:
    // (Get points to start of R5 memory after it's memory mapped)
    volatile void* pR5Base = mmap(0, MEM_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BTCM_ADDR);
    if (pR5Base == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return pR5Base;
}

void freeR5MmapAddr(volatile void* pR5Base)
{
    if (munmap((void*) pR5Base, MEM_LENGTH)) {
        perror("R5 munmap failed");
        exit(EXIT_FAILURE);
    }
}

void sharedMem_setValues(int *led_values) {
    // printf("Sharing memory with R5\n");

    // Get access to shared memory for my uses
    volatile void *pR5Base = getR5MmapAddr();

    unsigned long led_offset[8] = {LED_STRIP_OFFSET_0, 
                                LED_STRIP_OFFSET_1, 
                                LED_STRIP_OFFSET_2,
                                LED_STRIP_OFFSET_3,
                                LED_STRIP_OFFSET_4,
                                LED_STRIP_OFFSET_5,
                                LED_STRIP_OFFSET_6,
                                LED_STRIP_OFFSET_7,};

    // printf("From Linux, memory hold:\n");
    // NOTE: Cannot access it as a string, gives "Bus error"
    // printf("    %15s: \"%s\"\n", "msg", (char*)(pR5Base + MSG_OFFSET));
    


    // Drive it
    // Set LEDs
    for (int i = 0; i < 8; i++) {
        MEM_UINT32(pR5Base + led_offset[i]) = led_values[i];
    }

    // for (int i = 0; i < 8; i++) {
    //     printf("    %15s %i address: 0x%p\n", "ledstrip", i, (pR5Base + led_offset[i]));
    //     printf("    %15s %i: 0x%08x\n", "ledstrip", i, MEM_UINT32(pR5Base + led_offset[i]));
    // }
    // printf("\n");

    // Cleanup
    freeR5MmapAddr(pR5Base);
}