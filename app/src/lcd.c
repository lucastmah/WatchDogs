#include "lcd.h"
#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "hal/joystick.h"
#include "utils.h"
#include "nightLight.h"
#include "camera_controls.h"
#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>

#define BUFFER_SIZE 30
#define LCD_ON_LENGTH_MS 5000

struct menu_position {
    int x;
    int y;
};

static UWORD *s_fb;
static bool isInitialized = false;
static bool stop = false;
static _Atomic bool screen_on = false, wake_screen = false;
static long long last_on_event, start_time;

static pthread_t lcdThread;

static void lcd_updateScreen(char* message) {
    assert(isInitialized);

    const int x = 1;
    const int y = 100;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    // Draw into the RAM frame buffer
    // WARNING: Don't print strings with `\n`; will crash!
    Paint_DrawString_EN(x, y, message, &Font24, WHITE, BLACK);

    // Send the RAM frame buffer to the LCD (actually display it)
    // Option 1) Full screen refresh (~1 update / second)
    // LCD_1IN54_Display(s_fb);
    // Option 2) Update just a small window (~15 updates / second)
    //           Assume font height <= 20
    LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, s_fb);
}

void* lcd_Thread(void* args) {
    (void)args;

    time_t now;
    struct tm *timeinfo;
    long long time_now_s, time_now;

    while (!stop) {
        time_now = getTimeInMs();
        // Handle screen wake/sleep events
        if (wake_screen) {
	        LCD_SetBacklight(1023);
            last_on_event = time_now;
            screen_on = true;
            wake_screen = false;
        } else if (time_now > last_on_event + LCD_ON_LENGTH_MS) {
            LCD_SetBacklight(0);
            screen_on = false;
        }
        if (screen_on) {
            time(&now);
            timeinfo = localtime(&now);
            time_now_s = (time_now - start_time) / 1000;

            // Print to LCD
            char buff[1024];
            char time_buff[20];
            strftime(time_buff, sizeof(time_buff), "%Y-%m-%d    %H:%M:%S", timeinfo);
            snprintf(buff, 1024, "Current time: %s         Uptime: %02lld:%02lld", time_buff, time_now_s / 60, time_now_s % 60);
            lcd_updateScreen(buff);
        }
        sleepForMs(500);
    }
}

void lcd_wakeScreen(void) {
    wake_screen = true;
}

void lcd_init()
{
    assert(!isInitialized);

    // Exception handling:ctrl + c
    // signal(SIGINT, Handler_1IN54_LCD);
    
    // Module Init
	if(DEV_ModuleInit() != 0){
        DEV_ModuleExit();
        exit(0);
    }

    // LCD Init
    DEV_Delay_ms(2000);
	LCD_1IN54_Init(HORIZONTAL);
	LCD_1IN54_Clear(WHITE);
	// LCD_SetBacklight(1023);
    
    UDOUBLE Imagesize = LCD_1IN54_HEIGHT*LCD_1IN54_WIDTH*2;
    if((s_fb = (UWORD *)malloc(Imagesize)) == NULL) {
        perror("Failed to apply for black memory");
        exit(0);
    }
    isInitialized = true;
    start_time = getTimeInMs();

    if (pthread_create(&lcdThread, NULL, lcd_Thread, NULL) != 0) {
        perror("failed to create lcd thread");
        exit(EXIT_FAILURE);
    }
}
void lcd_cleanup()
{
    assert(isInitialized);
    stop = true;
    if (pthread_join(lcdThread, NULL) != 0) {
        perror("failed to join lcd thread");
        exit(EXIT_FAILURE);
    }
    LCD_SetBacklight(0);

    // Module Exit
    free(s_fb);
    s_fb = NULL;
	DEV_ModuleExit();
    isInitialized = false;
}