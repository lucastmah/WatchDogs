#include "lcd.h"
#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "hal/joystick.h"
#include "hal/timeout.h"
#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>

#define BUFFER_SIZE 30


static UWORD *s_fb;
static bool isInitialized = false;
static _Atomic int screen_mode = 0;
static long long startTime = 0;

static pthread_t lcdThread;
static pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;

static int menu_item = 0;

static void lcd_refreshGameScreen(void) 
{
    pthread_mutex_lock(&screen_mutex);
    {
        // draw indicator for currently selected menu item

        // draw menu items
        Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
        Paint_Clear(WHITE);

        int x = 30;
        int y = 10;
        Paint_DrawString_EN(x, y, "Zoom", &Font20, WHITE, BLACK);
        LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH - x, y + 20, s_fb);

        y += 20;
        Paint_DrawRectangle(x + 1, y + 1, LCD_1IN54_WIDTH - x, y + 19, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawRectangle(x + 3, y + 3, LCD_1IN54_WIDTH - x - 2, y + 18, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH - x, y + 20, s_fb);

        y += 30;
        Paint_DrawString_EN(x, y, "Volume", &Font20, WHITE, BLACK);
        LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH - x, y + 20, s_fb);

        y += 20;
        Paint_DrawRectangle(x + 1, y + 1, LCD_1IN54_WIDTH - x, y + 19, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawRectangle(x + 3, y + 3, LCD_1IN54_WIDTH - x - 2, y + 18, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH - x, y + 20, s_fb);

        y += 30;
        Paint_DrawString_EN(x, y, "Motion Light", &Font20, WHITE, BLACK);
        LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH - x, y + 20, s_fb);

        y += 20;
        Paint_DrawRectangle(x + 1, y + 1, LCD_1IN54_WIDTH - x, y + 19, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawRectangle(x + 3, y + 3, LCD_1IN54_WIDTH - x - 2, y + 18, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH - x, y + 20, s_fb);
    }
    pthread_mutex_unlock(&screen_mutex);
}

static void* lcd_displayRoutine()
{   
    while(isInitialized){
        lcd_refreshGameScreen();
        DEV_Delay_ms(100);
    }
    return NULL;
}

// void lcd_changeScreen(int new_screen_mode) {
//     assert(isInitialized);

//     // Initialize the RAM frame buffer to be blank (white)
//     pthread_mutex_lock(&screen_mutex);

//     Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
//     Paint_Clear(WHITE);

//     LCD_1IN54_Display(s_fb);
//     screen_mode = new_screen_mode % 3;

//     pthread_mutex_unlock(&screen_mutex);

//     lcd_refreshHomeScreen();

//     terminal_getAudioBufferStats(&stats);
//     lcd_refreshAudioScreen(stats);

//     terminal_getAccelerometerStats(&stats);
//     lcd_refreshAccelerometerScreen(stats);

//         // Send the RAM frame buffer to the LCD (actually display it)
//         // Option 1) Full screen refresh (~1 update / second)
//         // LCD_1IN54_Display(s_fb);
//         // Option 2) Update just a small window (~15 updates / second)
//         //           Assume font height <= 20
//         // LCD_1IN54_DisplayWindows(0, 0, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, s_fb);
	
//     #if 0
//     // Some other things you can do!

//     // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
//     Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
//     printf("Paint_Clear(WHITE)\n");
//     Paint_Clear(WHITE);
//     DEV_Delay_ms(2000);

// 	// Paint_SetRotate(ROTATE_90);
//     // Paint_SetRotate(ROTATE_180);
//     // Paint_SetRotate(ROTATE_270);

//     // /* GUI */
//     printf("drawing...\r\n");
//     // /*2.Drawing on the image*/
//     Paint_DrawPoint(5, 10, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);//240 240
//     Paint_DrawPoint(5, 25, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
//     Paint_DrawPoint(5, 40, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
//     Paint_DrawPoint(5, 55, BLACK, DOT_PIXEL_4X4, DOT_STYLE_DFT);

//     Paint_DrawLine(20, 10, 70, 60, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
//     Paint_DrawLine(70, 10, 20, 60, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
//     Paint_DrawLine(170, 15, 170, 55, RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
//     Paint_DrawLine(150, 35, 190, 35, RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

//     Paint_DrawRectangle(20, 10, 70, 60, BLUE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//     Paint_DrawRectangle(85, 10, 130, 60, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

//     Paint_DrawCircle(170, 35, 20, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(170, 85, 20, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);

//     Paint_DrawString_EN(5, 70, "hello world", &Font16, WHITE, BLACK);
//     Paint_DrawString_EN(5, 90, "waveshare", &Font20, RED, IMAGE_BACKGROUND);

//     Paint_DrawNum(5, 160, 123456789, &Font20, GREEN, IMAGE_BACKGROUND);
// 	Paint_DrawString_CN(5,200, "΢ѩ����",  &Font24CN,IMAGE_BACKGROUND,BLUE);   
    
//     // /*3.Refresh the picture in RAM to LCD*/
//     LCD_1IN54_Display(s_fb);
// 	DEV_Delay_ms(2000);

//     // /* show bmp */
// 	printf("show bmp\r\n");	
// 	GUI_ReadBmp("./pic/LCD_1inch54.bmp");    
//     LCD_1IN54_Display(s_fb);
//     DEV_Delay_ms(2000);
//     #endif
// }

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
	LCD_SetBacklight(1023);
    
    UDOUBLE Imagesize = LCD_1IN54_HEIGHT*LCD_1IN54_WIDTH*2;
    if((s_fb = (UWORD *)malloc(Imagesize)) == NULL) {
        perror("Failed to apply for black memory");
        exit(0);
    }
    isInitialized = true;
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    if (pthread_create(&lcdThread, NULL, lcd_displayRoutine, NULL) != 0) {
        perror("failed to create lcd thread");
        exit(EXIT_FAILURE);
    }
}
void lcd_cleanup()
{
    assert(isInitialized);
    isInitialized = false;
    if (pthread_join(lcdThread, NULL) != 0) {
        perror("failed to join lcd thread");
        exit(EXIT_FAILURE);
    }

    // Module Exit
    free(s_fb);
    s_fb = NULL;
    LCD_1IN54_Clear(BLACK);
	DEV_ModuleExit();
}