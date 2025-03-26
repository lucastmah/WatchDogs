#include "hal/timeout.h"
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static void* timer(void* args) {
    struct timespec delay = {0, 150000000};
    nanosleep(&delay, (struct timespec*) NULL);
    *(_Atomic bool *) args = false;
    pthread_exit(0);
}

void timeout_start_timer(_Atomic bool* ptr) {
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, timer, ptr) != 0) {
        perror("failed to create timeout thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_detach(thread_id) != 0) {
        perror("failed to detach timeout thread");
        exit(EXIT_FAILURE);
    }
}