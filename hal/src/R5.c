#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <string.h>

#include "hal/sharedDataLayout.h"
#include "hal/R5.h"

// General R5 Memomry Sharing Routine
// ----------------------------------------------------------------
#define ATCM_ADDR     0x79000000  // MCU ATCM (p59 TRM)
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

// COLOURS
// - 1st element in array is 1st (bottom) on LED strip; last element is last on strip (top)
// - Bits: {Green/8 bits} {Red/8 bits} {Blue/8 bits} {White/8 bits}
uint32_t colour[COUNT] = {
    0x0f000000, // Green
    0x000f0000, // Red
    0x00000f00, // Blue
    // 0x0000000f, // White
    // 0x0f0f0f00, // White (via RGB)

    0x0fff0000, // Orange

    0x50ff0000, // Yellow
    // 0x000f0f00, // Purple
    // 0x0f000f00, // Teal

    // Try these; they are birght! 
    // (You'll need to comment out some of the above)
    0xff000000, // Green Bright
    0x00ff0000, // Red Bright
    0x0000ff00, // Blue Bright
    // 0xffffff00, // White
    // 0xff0000ff, // Green Bright w/ Bright White
    // 0x00ff00ff, // Red Bright w/ Bright White
    // 0x0000ffff, // Blue Bright w/ Bright White
    // 0xffffffff, // White w/ Bright White

    0x00000000, // Black
};

volatile void *pR5Base;

void R5_init(void) {
    printf("Sharing memory with R5\n");
    // printf("  LED should change speed every 5s.\n");
    // printf("  Press the button to see its state here.\n");

    // Get access to shared memory for my uses
    pR5Base = getR5MmapAddr();
}

void R5_cleanup(void) {
    for (int i = 0; i < NEO_NUM_LEDS; i++) {
		setSharedMem_uint32(pR5Base, NEOPIXEL_LEDS_OFFSET + i * sizeof(uint32_t), 0);
	}

    // Cleanup
    freeR5MmapAddr(pR5Base);
}

// bool R5_getButtonState() {
//     return getSharedMem_uint32(pR5Base, IS_BUTTON_PRESSED_OFFSET);
// }

bool R5_getJoystickButtonState() {
    return getSharedMem_uint32(pR5Base, IS_JOY_BUTTON_PRESSED_OFFSET);
}

void R5_setLEDs(enum Colour input[]) 
{
    for (int j = 0; j < NEO_NUM_LEDS; j++) {
        setSharedMem_uint32(pR5Base, NEOPIXEL_LEDS_OFFSET + j * sizeof(uint32_t), colour[input[j]]);
    }
}