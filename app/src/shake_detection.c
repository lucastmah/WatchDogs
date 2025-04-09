#include "shake_detection.h"
#include "sendMail.h"
#include "hal/accelerometer.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define AGGRESSIVE_SHAKE 12000
// send no more than 1 per hour
#define EMAIL_LOCKOUT_TIME 3600000
#define MAX_EMAIL_LENGTH 100
#define SHAKE_DEBOUNCE_MS 150

static bool isInitialized = false;
static pthread_t thread_id;

static struct accelerometer_values last_values;
static long long last_time_email_sent, last_shake_event;

char email[MAX_EMAIL_LENGTH+1] = {0};

static void checkForEmailSend(int curr, int last) {
    long long current_time = getTimeInMs();
    if (current_time < last_shake_event + SHAKE_DEBOUNCE_MS) {
        return;
    }
    if (abs(curr - last) > AGGRESSIVE_SHAKE) {
        last_shake_event = current_time;
        if (current_time - last_time_email_sent > EMAIL_LOCKOUT_TIME) {
            last_time_email_sent = current_time;
            if (email[0]) {
                sendMail_send(email);
                printf("Sending email to %s\n", email);
            } else {
                printf("No email defined\n");
            }
        }
        printf("Camera is being attacked.\n");
    }
}

static void* shakeDetect_loop() {
    while (isInitialized) {
        struct accelerometer_values values;
        accelerometer_getValues(&values);
        checkForEmailSend(values.x, last_values.x);
        last_values.x = values.x;
        checkForEmailSend(values.y, last_values.y);
        last_values.y = values.y;
        checkForEmailSend(values.z, last_values.z);
        last_values.z = values.z;
        sleepForMs(5);
    }
    return NULL;
}

void shakeDetect_setEmail(char input[]) {
    strncpy(email, input, MAX_EMAIL_LENGTH);
    email[MAX_EMAIL_LENGTH] = 0;
}

void shakeDetect_init(void) {
    assert(!isInitialized);
    isInitialized = true;
    // flush out accelerometer values for accurate reading
    for (int i = 0; i < 5; i++) {
        accelerometer_getValues(&last_values);
    }
    last_time_email_sent = getTimeInMs() - EMAIL_LOCKOUT_TIME;
    if (pthread_create(&thread_id, NULL, shakeDetect_loop, NULL) != 0) {
        perror("failed to create shakeDetect thread");
        exit(EXIT_FAILURE);
    }
}

void shakeDetect_cleanup(void) {
    assert(isInitialized);
    isInitialized = false;
    if (pthread_join(thread_id, NULL) != 0) {
        perror("failed to join shakeDetect thread");
        exit(EXIT_FAILURE);
    }
}