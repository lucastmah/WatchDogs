/*
 * R5 Sample Code for Shared Memory with Linux
 */
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#include "sharedDataLayout.h"

#define NEO_NUM_LEDS          8   // # LEDs in our string

// Memory
// ----------------------------------------
#define SHARED_MEM_BTCM_START 0x00000000  // TRM p848
#define SHARED_MEM_ATCM_START 0x00041010  // TRM p849
#define BASE ((void*)(SHARED_MEM_BTCM_START))

// Access GPIO (for demonstration purposes)
// ----------------------------------------
// 1,000,000 uSec = 1000 msec = 1 sec
#define MICRO_SECONDS_PER_MILI_SECOND   (1000)
#define DEFAULT_LED_DELAY_MS            (100)

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
// #define LED0_NODE DT_ALIAS(led0)
// #define BTN0_NODE DT_ALIAS(btn0)
#define NEOPIXEL_NODE DT_ALIAS(neopixel)
#define BTN1_NODE DT_ALIAS(btn1)

// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
// static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
static const struct gpio_dt_spec neopixel = GPIO_DT_SPEC_GET(NEOPIXEL_NODE, gpios);
static const struct gpio_dt_spec jbtn = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);

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

	// initialize_gpio(&led, GPIO_OUTPUT_ACTIVE);
	// initialize_gpio(&btn, GPIO_INPUT);
	initialize_gpio(&neopixel, GPIO_OUTPUT_ACTIVE);
	initialize_gpio(&jbtn, GPIO_INPUT);

	// printf("Contents of Shared Memory BTCM:\n");
	// for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
	// 	uint8_t val = getSharedMem_uint8(BASE, i);
	// 	printf("0x%08x = %2x (%c)\n", i, val, val);
	// }

	// Setup defaults
	// printf("Writing to BTCM...\n");
	// char* msg = "Hello from R5 World (Take 7)!";
	// for (int i = 0; i < strlen(msg); i++) {
	// 	setSharedMem_uint8(BASE, MSG_OFFSET + i, msg[i]);
	// }
	// for (int i = 0; i < NEO_NUM_LEDS; i++) {
	// 	setSharedMem_uint32(BASE, NEOPIXEL_LEDS_OFFSET + i * sizeof(uint32_t), 0);
	// }
	
	// setSharedMem_uint32(BASE, LED_DELAY_MS_OFFSET, DEFAULT_LED_DELAY_MS);
	// setSharedMem_uint32(BASE, IS_BUTTON_PRESSED_OFFSET, 0);
	setSharedMem_uint32(BASE, IS_JOY_BUTTON_PRESSED_OFFSET, 0);
	// setSharedMem_uint32(BASE, BTN_COUNT_OFFSET, 0);
	// setSharedMem_uint32(BASE, LOOP_COUNT_OFFSET, 0);
	
	// printf("Contents of Shared Memory BTCM After Write:\n");
	// for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
	// 	uint8_t val = getSharedMem_uint8(BASE, i);
	// 	printf("0x%08x = %2x (%c)\n", i, val, val);
	// }
	while (true) {
		gpio_pin_set_dt(&neopixel, 0);
		DELAY_NS(NEO_RESET_NS);

		for(int j = 0; j < NEO_NUM_LEDS; j++) {
			uint32_t colour = getSharedMem_uint32(BASE, NEOPIXEL_LEDS_OFFSET + j * sizeof(uint32_t));
			for(int i = 31; i >= 0; i--) {
				if(colour & ((uint32_t)0x1 << i)) {
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

		// Read GPIO state and share with Linux
		// int state = gpio_pin_get_dt(&btn);
		// bool isPressed = state == 0;
		int joystickState = gpio_pin_get_dt(&jbtn);
		bool isJoystickPressed = joystickState == 0;

		// Update shared memory to Linux
		// setSharedMem_uint32(BASE, IS_BUTTON_PRESSED_OFFSET, isPressed);
		setSharedMem_uint32(BASE, IS_JOY_BUTTON_PRESSED_OFFSET, isJoystickPressed);
	}
	return 0;
}
