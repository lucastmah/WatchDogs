// NeoPixel Driver Sample
//
// Based off the Zephyr blinky sample application.
// - Designed to be compiled for BeagleY-AI's MCU R5
//   (because the custom hardware uses pins that are mapped to the MCU domain)

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#include "sharedDataLayout.h"

// Memory
// ----------------------------------------
// #define SHARED_MEM_BTCM_START 0x00000000  // TRM p848
// #define SHARED_MEM_ATCM_START 0x00041010  // TRM p849
// static void *pSharedMem = (void *) SHARED_MEM_BTCM_START;

// Access GPIO (for demonstration purposes)
// ----------------------------------------
// 1,000,000 uSec = 1000 msec = 1 sec
#define MICRO_SECONDS_PER_MILI_SECOND   (1000)
#define DEFAULT_LED_DELAY_MS            (100)

// Device tree nodes for pin aliases
#define LED0_NODE DT_ALIAS(led0)
#define BTN0_NODE DT_ALIAS(btn0)

// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
// static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);

#define NEO_NUM_LEDS          8   // # LEDs in our string
#define NEO_NUM_COLORS 9

// NeoPixel Timing
// NEO_<one/zero>_<on/off>_NS
// (These times are what the hardware needs; the delays below are hand-tuned to give these).
#define NEO_ONE_ON_NS       700   // Stay on 700ns
#define NEO_ONE_OFF_NS      600   // (was 800)
#define NEO_ZERO_ON_NS      350
#define NEO_ZERO_OFF_NS     800   // (Was 600)
#define NEO_RESET_NS      60000   // Must be at least 50us, use 60us

// Delay time includes 1 GPIO set action.
volatile int junk_delay = 0;
#define DELAY_350_NS() {}
#define DELAY_600_NS() {for (junk_delay=0; junk_delay<9 ;junk_delay++);}
#define DELAY_700_NS() {for (junk_delay=0; junk_delay<16 ;junk_delay++);}
#define DELAY_800_NS() {for (junk_delay=0; junk_delay<23 ;junk_delay++);}

#define DELAY_NS(ns) do {int target = k_cycle_get_32() + k_ns_to_cyc_near32(ns); \
	while(k_cycle_get_32() < target) ; } while(false)

#define NEO_DELAY_ONE_ON()     DELAY_700_NS()
#define NEO_DELAY_ONE_OFF()    DELAY_600_NS()
#define NEO_DELAY_ZERO_ON()    DELAY_350_NS()
#define NEO_DELAY_ZERO_OFF()   DELAY_800_NS()
#define NEO_DELAY_RESET()      {DELAY_NS(NEO_RESET_NS);}

// Device tree nodes for pin aliases
#define LED0_NODE DT_ALIAS(led0)
#define BTN0_NODE DT_ALIAS(btn0)
#define NEOPIXEL_NODE DT_ALIAS(neopixel)

// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
// static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
static const struct gpio_dt_spec neopixel = GPIO_DT_SPEC_GET(NEOPIXEL_NODE, gpios);

static void initialize_gpio(const struct gpio_dt_spec *pPin, int direction) 
{
	if (!gpio_is_ready_dt(pPin)) {
		printf("ERROR: GPIO pin not ready read; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}

	int ret = gpio_pin_configure_dt(pPin, direction);
	if (ret < 0) {
		printf("ERROR: GPIO Pin Configure issue; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	initialize_gpio(&neopixel, GPIO_OUTPUT_ACTIVE);

	// Addresses:
	unsigned long led_offset[8] = {LED_STRIP_OFFSET_0, 
		LED_STRIP_OFFSET_1, 
		LED_STRIP_OFFSET_2,
		LED_STRIP_OFFSET_3,
		LED_STRIP_OFFSET_4,
		LED_STRIP_OFFSET_5,
		LED_STRIP_OFFSET_6,
		LED_STRIP_OFFSET_7,};

	// Print out the mem contents:
    // printf("From the R5, memory hold:\n");
    // for (int i = 0; i < 8; i++) {
    //     printf("    led_%i: 0x%08x\n", i, led_offset[i]);   
    // }

	// printf("Contents of Shared Memory ATCM:\n");
	// for (int i = 0; i < END_MEMORY_OFFSET; i++) {
	// 	char* addr = (char*)pSharedMem + i;
	// 	printf("0x%08x = %2x (%c)\n", (uint32_t) addr, *addr, *addr);
	// }

	// Setup defaults
	// printf("Writing to ATCM...\n");
	// printf("   ..1\n");
	// strcpy((char*) MSG_OFFSET, "Hello from R5 World (Take 6)!");

	// printf("Contents of Shared Memory BTCM after string init:\n");
	// for (int i = 0; i < END_MEMORY_OFFSET; i++) {
	// 	char* addr = (char*)pSharedMem + i;
	// 	printf("0x%08x = %2x (%c)\n", (uint32_t) addr, *addr, *addr);
	// }

	// bool led_state = true;
	// uint32_t btnCount = 0;
	// uint32_t loopCount = 0;
	// while (true) {
	// 	// Toggle LED
	// 	printf("LED state: %s\n", led_state ? "OFF" : "ON");
	// 	if (gpio_pin_toggle_dt(&led) < 0) {
	// 		printf("ERROR: GPIO Pin Toggle DT\n");
	// 		return 0;
	// 	}
	// 	led_state = !led_state;


	// 	// Read GPIO state and share with Linux
	// 	int state = gpio_pin_get_dt(&btn);
	// 	bool isPressed = state == 0;
	// 	printf("Is button pressed? %d\n", isPressed);

	// 	// Update shared count of butten pressed
	// 	if (isPressed) {
	// 		btnCount++;
	// 	}
	// 	loopCount++;

	// 	// Update shared memory to Linux
	// 	MEM_UINT32(IS_BUTTON_PRESSED_OFFSET) = isPressed;
	// 	MEM_UINT32(LOOP_COUNT_OFFSET) = loopCount;
	// 	MEM_UINT32(BTN_COUNT_OFFSET) = btnCount;

	// 	// Wait for delay (set by Linux app)
	// 	uint32_t delay = MEM_UINT32(LED_DELAY_MS_OFFSET);
	// 	printf("Waiting for %d ms\n", delay);
	// 	k_busy_wait(delay * MICRO_SECONDS_PER_MILI_SECOND);	
	// }

	// enum LED_VALUES {
	// 	RED,
	// 	RED_BRIGHT,
	// 	GREEN,
	// 	GREEN_BRIGHT,
	// 	BLUE, 
	// 	BLUE_BRIGHT,
	// 	OFF,
	// };
	
	// COLOURS
    // - 1st element in array is 1st (bottom) on LED strip; last element is last on strip (top)
    // - Bits: {Green/8 bits} {Red/8 bits} {Blue/8 bits} {White/8 bits}
    uint32_t color[NEO_NUM_COLORS] = {
		0x000f0000, // Red
		0x00ff0000, // Red Bright
		0x0f000000, // Green
		0xff000000, // Green Bright
		0x00000f00, // Blue
		0x0000ff00, // Blue Bright
		0x00000000, // Off
        // 0x0f000000, // Green
        // 0x000f0000, // Red
        // 0x00000f00, // Blue
        // 0x0000000f, // White
        // 0x0f0f0f00, // White (via RGB)
        0x0f0f0000, // Yellow
        // 0x000f0f00, // Purple
        0x0f000f00, // Teal

        // Try these; they are birght! 
        // (You'll need to comment out some of the above)
        // 0xff000000, // Green Bright
        // 0x00ff0000, // Red Bright
        // 0x0000ff00, // Blue Bright
        // 0xffffff00, // White
        // 0xff0000ff, // Green Bright w/ Bright White
        // 0x00ff00ff, // Red Bright w/ Bright White
        // 0x0000ffff, // Blue Bright w/ Bright White
        // 0xffffffff, // White w/ Bright White
    };    


	while (true) {
		gpio_pin_set_dt(&neopixel, 0);
		DELAY_NS(NEO_RESET_NS);

		for(int j = 0; j < NEO_NUM_LEDS; j++) {
			for(int i = 31; i >= 0; i--) {
				int val = (int)MEM_UINT32(led_offset[j]);
				if(color[val] & ((uint32_t)0x1 << i)) {
				// if(color[j] & ((uint32_t)0x1 << i)) {
					gpio_pin_set_dt(&neopixel, 1);
					NEO_DELAY_ONE_ON();
					gpio_pin_set_dt(&neopixel, 0);
					NEO_DELAY_ONE_OFF();
				} else {
					gpio_pin_set_dt(&neopixel, 1);
					NEO_DELAY_ZERO_ON();
					gpio_pin_set_dt(&neopixel, 0);
					NEO_DELAY_ZERO_OFF();
				}
			}
		}

		gpio_pin_set_dt(&neopixel, 0);
		NEO_DELAY_RESET();

		// Keep looping in case we plug in NeoPixel later
		k_busy_wait(1 * 10000);
	}
	return 0;
}