#include "menu.h"
#include "hal/rotary.h"
#include "lcd.h"
#include "nightLight.h"
#include "camera_controls.h"
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define MENU_ITEM_COUNT 2

// Menu items: Zoom, Volume, Motion Light, Mute, Patrol
static int menuItem = 0;
static bool selected = false;
static int priorTurnCounter = 0;

static _Atomic bool isInitialized = false;

void menu_processTurnState(int turnCounter) {
    bool currState;
    assert(isInitialized);

    int change = turnCounter - priorTurnCounter;
    priorTurnCounter = turnCounter;

    // if an item is not selected, move between menu items
    if (!selected) {
        menuItem = menuItem + change;
        if (menuItem < 0) {
            menuItem = 0;
        }
        else if (menuItem > MENU_ITEM_COUNT - 1) {
            menuItem = MENU_ITEM_COUNT - 1;
        }
    }
    else {
        switch (menuItem) {
            case 0:
                currState = nightLight_getLightMode();
                if (!currState && change > 0) {
                    nightLight_setLightMode(true);
                }
                if (currState && change < 0) {
                    nightLight_setLightMode(false);
                }
                break;
            case 1:
                // currState = CameraControls_getPatrolMode();
                // if (!currState && change > 0) {
                //     CameraControls_setPatrolMode(true);
                // }
                // if (currState && change < 0) {
                //     CameraControls_setPatrolMode(false);
                // }
                break;
            default:
                printf("shouldn't happen\n");
        }
    }

    // refresh screen
    lcd_refreshGameScreen(menuItem, selected);
}

void menu_processPushState(int pushCount) {
    assert(isInitialized);

    selected = !selected;

    // refresh screen
    lcd_refreshGameScreen(menuItem, selected);
}

void menu_init(void) {
    assert(!isInitialized);
    isInitialized = true;
    rotary_addKnobSubscriber(menu_processTurnState);
    rotary_addPushSubscriber(menu_processPushState);

    lcd_refreshGameScreen(menuItem, selected);
}