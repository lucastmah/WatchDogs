#include "hal/led.h"

static bool initialized = false;

static char* ledFiles[] = {"PWR", "ACT"};

static FILE* led_openFile(char *file) {
    FILE *triggerFile = NULL;
    triggerFile = fopen(file, "w");
    if (triggerFile == NULL) {
        perror("Failed to open file");
        printf("%s\n", file);
        exit(EXIT_FAILURE);
    }
    return triggerFile;
}

static void led_setTrigger(enum LED led, char *mode) {
    char filePath[100];
    snprintf(filePath, sizeof(filePath), "/sys/class/leds/%s/trigger", ledFiles[led]);
    FILE *triggerFile = led_openFile(filePath);
    int updateStatus = fprintf(triggerFile, "%s", mode);
    if (updateStatus <= 0) {
        perror("Trigger file error");
        exit(EXIT_FAILURE);
    }
    fclose(triggerFile);
}

// Set trigger = None for all LEDs
void led_initialize() {
    if (initialized) {
        perror("Initialized leds more than once");
        exit(EXIT_FAILURE);
    }
    int num_of_leds = sizeof(ledFiles) / sizeof(ledFiles[0]);
    for (int i = 0; i < num_of_leds; i++) {
        led_setTrigger(i, "none");
    }
    initialized = true;
}

// Set trigger = None for all LEDs
void led_cleanup() {
    if (!initialized) {
        return;
    }
    int num_of_leds = sizeof(ledFiles) / sizeof(ledFiles[0]);
    for (int i = 0; i < num_of_leds; i++) {
        led_setTrigger(i, "none");
    }
    initialized = false;
}

void led_reset(enum LED led) {
    led_setTrigger(led, "none");
}

void led_setBrightness(enum LED led, bool activation) {
    assert(initialized);

    char filePath[100];
    snprintf(filePath, sizeof(filePath), "/sys/class/leds/%s/brightness", ledFiles[led]);

    FILE *brightnessFile = led_openFile(filePath);
    int updateStatus = 1;
    if (activation) {
        updateStatus = fprintf(brightnessFile, "1");
    }
    else {
        updateStatus = fprintf(brightnessFile, "0");
    }
    if (updateStatus <= 0) {
        perror("Error writing to brightness file.\n");
        exit(EXIT_FAILURE);
    }
    fclose(brightnessFile);
}

void led_setBlinkInterval(enum LED led, int time_on, int time_off) {
    char filePath[100];
    
    assert(initialized);

    led_setTrigger(led, "timer");

    snprintf(filePath, sizeof(filePath), "/sys/class/leds/%s/delay_off", ledFiles[led]);

    FILE *delayOffFile = led_openFile(filePath);

    int offUpdateStatus = fprintf(delayOffFile, "%d", time_off);
    if (offUpdateStatus <= 0) {
        perror("Delay off file error");
        exit(EXIT_FAILURE);
    }
    fclose(delayOffFile);

    snprintf(filePath, sizeof(filePath), "/sys/class/leds/%s/delay_on", ledFiles[led]);

    FILE *delayOnFile = led_openFile(filePath);
    int onUpdateStatus = fprintf(delayOnFile, "%d", time_on);
    if (onUpdateStatus <= 0) {
        perror("Delay on file error");
        exit(EXIT_FAILURE);
    }
    fclose(delayOnFile);
}