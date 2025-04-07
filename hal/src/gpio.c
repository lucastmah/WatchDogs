#include "hal/gpio.h"
#include <gpiod.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

// Relies on the gpiod library.
// Insallation for cross compiling:
//      (host)$ sudo dpkg --add-architecture arm64
//      (host)$ sudo apt update
//      (host)$ sudo apt install libgpdiod-dev:arm64
// GPIO: https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library
// Example: https://github.com/starnight/libgpiod-example/blob/master/libgpiod-input/main.c

// TYPE NOTE:
// Internally cast the 
//    struct GpioLine* 
// to 
//    (struct gpiod_line*)
// so we hide the dependency on gpiod

#define MAX_EVENT_COUNT 1000
#define MAX_GPIO_LINES 10

static _Atomic bool s_isInitialized = false;
static pthread_t thread_ids[GPIO_NUM_CHIPS];

static char* s_chipNames[] = {
    "gpiochip0",
    "gpiochip1",
    "gpiochip2",
};

static struct gpiolines gpio_lines[MAX_GPIO_LINES];
struct gpiod_line* lines[MAX_GPIO_LINES];
static int gpio_lines_count = 0;

struct gpiod_line_bulk bulkWait[GPIO_NUM_CHIPS];

// Hold open chips
static struct gpiod_chip* s_openGpiodChips[GPIO_NUM_CHIPS];

static void* gpio_loop(void* args) {
    struct timespec delay = {1, 0};
    unsigned int chipNum = *(unsigned int *)args;
    free(args);

    while(s_isInitialized) {
        struct gpiod_line_bulk bulkEvents;
        // watches the status of the event queues
        int result = gpiod_line_event_wait_bulk(&bulkWait[chipNum], &delay, &bulkEvents);
        if (result == -1) {
            perror("Error with event waiting");
            exit(EXIT_FAILURE);
        }
        if (result == 0) {
            continue;
        }
        int num_of_events = gpiod_line_bulk_num_lines(&bulkEvents);
            
        for (int i = 0; i < num_of_events; i++) {
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            unsigned int this_line_number = gpiod_line_offset(line_handle);

            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle, &event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }
            
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
            for (int j = 0; j < gpio_lines_count; j++) {
                if (chipNum == gpio_lines[j].chip && this_line_number == gpio_lines[j].pin) {
                    gpio_lines[j].action(chipNum, this_line_number, isRising);
                }
            }
        }
    }

    return NULL;
}

// Opening a pin gives us a "line" that we later work with.
//  chip: such as GPIO_CHIP_0
//  pinNumber: such as 15
static struct gpiod_line* Gpio_openForEvents(enum eGpioChips chip, int pinNumber)
{
    assert(s_isInitialized);
    struct gpiod_chip* gpiodChip = s_openGpiodChips[chip];
    struct gpiod_line* line = gpiod_chip_get_line(gpiodChip, pinNumber);
    if (!line) {
        perror("Unable to get GPIO line");
        exit(EXIT_FAILURE);
    }

    return line;
}


// Initialize only after adding all desired gpio lines
// Add a gpio line to be monitored with Gpio_addLineToBulk
void Gpio_initialize(void)
{
    assert(!s_isInitialized);
    s_isInitialized = true;

    // Open GPIO Chips
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
         // Open GPIO chip
        s_openGpiodChips[i] = gpiod_chip_open_by_name(s_chipNames[i]);
        if (!s_openGpiodChips[i]) {
            perror("GPIO Initializing: Unable to open a GPIO chip");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize pointers to get callbacks for GPIO events
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
        gpiod_line_bulk_init(&bulkWait[i]);
    }

    // Add lines to corresponding bulkWait to GPIO Chip
    for (int i = 0; i < gpio_lines_count; i++) {
        lines[i] = Gpio_openForEvents(gpio_lines[i].chip, gpio_lines[i].pin);
        gpiod_line_bulk_add(&bulkWait[gpio_lines[i].chip], lines[i]);
    }
    
    // Start a thread for each bulkWait that has at least one line in it
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
        if (gpiod_line_bulk_num_lines(&bulkWait[i]) > 0) {
            gpiod_line_request_bulk_both_edges_events(&bulkWait[i], "Event Waiting");
            unsigned int* var = malloc(sizeof(*var));
            *var = i;
            if(pthread_create(&thread_ids[i], NULL, gpio_loop, var) != 0) {
                perror("failed to start a gpio waiting thread");
                exit(EXIT_FAILURE);
            }        
        }
    }
}

void Gpio_cleanup(void)
{
    assert(s_isInitialized);
    s_isInitialized = false;

    // Rejoin threads
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
        if (gpiod_line_bulk_num_lines(&bulkWait[i]) > 0) {
            pthread_join(thread_ids[i], NULL);
        }
    }

    // Close opened gpio lines
    for (int i = 0; i < GPIO_NUM_LINES; i++) {
        Gpio_close(lines[i]);    
    }

    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
        // Close GPIO chip
        gpiod_chip_close(s_openGpiodChips[i]);
        if (!s_openGpiodChips[i]) {
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
}

void Gpio_addLineToBulk(int chip, int pin, void (*action)(int chip, int pin, bool is_rising)) {
    assert(!s_isInitialized);
    gpio_lines[gpio_lines_count] = (struct gpiolines) {chip, pin, action};
    gpio_lines_count++;
}

void Gpio_close(struct gpiod_line* line)
{
    assert(s_isInitialized);
    gpiod_line_release(line);
}