#include "shake.h"
#include "sendMail.h"
#include "hal/accelerometer.h"
#include "hal/timeout.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define AGGRESSIVE_SHAKE 14000
// send no more than 1 per hour
#define EMAIL_LOCKOUT_TIME 3600000

static _Atomic bool isInitialized = false;
static pthread_t thread_id;
static _Atomic bool lockoutAttackEvent = false;

static struct accelerometer_values last_values;
static long long last_time_email_sent;

static void checkForEmailSend(int curr, int last) {
    if (!lockoutAttackEvent) {
        long long current_time = getTimeInMs();
        if (abs(curr - last) > AGGRESSIVE_SHAKE) {
            if (current_time - last_time_email_sent > EMAIL_LOCKOUT_TIME) {
                last_time_email_sent = current_time;
                // sendMail_send("lucastmah@gmail.com");
            }
            printf("Camera is being attacked.\n");
            lockoutAttackEvent = true;
            timeout_start_timer(&lockoutAttackEvent);
        }
    }
}

static void* shake_loop() {
    while (isInitialized) {
        struct accelerometer_values values;
        accelerometer_getValues(&values);
        checkForEmailSend(values.x, last_values.x);
        last_values.x = values.x;
        checkForEmailSend(values.y, last_values.y);
        last_values.y = values.y;
        checkForEmailSend(values.z, last_values.z);
        last_values.z = values.z;
    }
    return NULL;
}

void shake_init(void) {
    assert(!isInitialized);
    isInitialized = true;
    // flush out accelerometer values for accurate reading
    for (int i = 0; i < 5; i++) {
        accelerometer_getValues(&last_values);
    }
    last_time_email_sent = getTimeInMs() - EMAIL_LOCKOUT_TIME;
    if (pthread_create(&thread_id, NULL, shake_loop, NULL) != 0) {
        perror("failed to create shake thread");
        exit(EXIT_FAILURE);
    }
}

void shake_cleanup(void) {
    assert(isInitialized);
    isInitialized = false;
    if (pthread_join(thread_id, NULL) != 0) {
        perror("failed to join shake thread");
        exit(EXIT_FAILURE);
    }
}