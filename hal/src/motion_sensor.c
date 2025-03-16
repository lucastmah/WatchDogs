#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <gpiod.h>
#include "hal/gpio.h"
#include "hal/led.h"

#define SENSOR_PIN 38
#define MAX_SUBSCRIBERS 5

// static pthread_t thread_id;
// static _Atomic bool motion_sensor_initialized;
static _Atomic bool motion_detected = false;

// static void* motion_sensor_loop() {
//     struct timespec delay = {1, 0};

//     assert(motion_sensor_initialized);
//     struct gpiod_line* line = Gpio_openForEvents(GPIO_CHIP_1, SENSOR_PIN);

//     gpiod_line_request_both_edges_events(line, "Motion Sensor Waiting");
//     while(motion_sensor_initialized) {
//         int result = gpiod_line_event_wait(line, &delay);
//         if(result == -1) {
//             perror("gpiod error for motion sensor");
//             exit(EXIT_FAILURE);
//         }
//         if(result == 0) {
//             continue;
//         }

//         struct gpiod_line_event event;
//         if (gpiod_line_event_read(line, &event) == -1) {
//             perror("Line Event");
//             exit(EXIT_FAILURE);
//         }

//         // Run the state machine
//         motion_detected = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
//         led_setBrightness(BYAI_GREEN, motion_detected);
//     }
//     return NULL;
// }

// void motion_sensor_initialize(void) {
//     assert(!motion_sensor_initialized);
//     motion_sensor_initialized = true;
//     led_initialize();
//     Gpio_initialize();
//     pthread_create(&thread_id, NULL, motion_sensor_loop, NULL);
// }

// void motion_sensor_cleanup(void) {
//     assert(motion_sensor_initialized);
//     motion_sensor_initialized = false;
//     pthread_join(thread_id, NULL);
//     Gpio_cleanup();
//     led_cleanup();
// }

static void (*subscribers[MAX_SUBSCRIBERS]) (bool motion_state);
static int sub_count = 0;
static pthread_mutex_t sub_mutex = PTHREAD_MUTEX_INITIALIZER;

void motionSensor_addSubscriber(void (*callback)(bool motion_state)) {
    if (sub_count < MAX_SUBSCRIBERS) {
        pthread_mutex_lock(&sub_mutex);
        subscribers[sub_count] = callback;
        pthread_mutex_unlock(&sub_mutex);
        sub_count++;
    }
}

void motionSensor_processState(int chip, int pin, bool is_rising) {
    if (chip == GPIO_CHIP_1 && pin == SENSOR_PIN) {
        if (is_rising) {
            motion_detected = true;
        }
        else {
            motion_detected = false;
        }
        for (int i = 0; i < sub_count; i++) {
            pthread_mutex_lock(&sub_mutex);
            subscribers[i](motion_detected);
            pthread_mutex_unlock(&sub_mutex);
        }
    }
}