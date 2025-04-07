#include "hal/rotary.h"
#include "hal/timeout.h"
#include "hal/gpio.h"
#include <pthread.h>
#include <assert.h>

#define MAX_SUBSCRIBERS 5

static _Atomic bool isInitialized = false;

static _Atomic int knob_counter = 0;
static _Atomic bool is_cw = true;

static _Atomic int push_counter = 0;
static _Atomic bool fell = false;
static _Atomic bool lock_push = false;

static void (*pushSubscribers[MAX_SUBSCRIBERS]) (int push_count);
static int push_sub_count = 0;

static void (*knobSubscribers[MAX_SUBSCRIBERS]) (int turn_counter);
static int knob_sub_count = 0;

/*
    Define the Statemachine Data Structures
*/
struct stateKnobEvent {
    struct knobState* pNextState;
    void (*action)();
};

struct edgeState {
    struct stateKnobEvent rising;
    struct stateKnobEvent falling;
};

struct knobState {
    struct edgeState A;
    struct edgeState B;
};

struct statePushEvent {
    struct pushState* pNextState;
    void (*action)();
};

struct pushState {
    struct statePushEvent rising;
    struct statePushEvent falling;
};

static void notifyPushSubscribers(void) {
    // Notify subscribers of new change
    assert(isInitialized);
    for (int i = 0; i < push_sub_count; i++) {
        pushSubscribers[i](push_counter);
    }
}

static void notifyKnobSubscribers(void) {
    // Notify subscribers of new change
    assert(isInitialized);
    for (int i = 0; i < knob_sub_count; i++) {
        knobSubscribers[i](knob_counter);
    }
}

/*
    START STATEMACHINE
*/
static void check_ccw_turn(void)
{   
    assert(isInitialized);
    if(!is_cw) {
        knob_counter--;
        notifyKnobSubscribers();
    }
}

static void check_cw_turn(void)
{   
    assert(isInitialized);
    if(is_cw) {
        knob_counter++;
        notifyKnobSubscribers();
    }
}

static void set_cw(void)
{
    is_cw = true;
}

static void set_ccw(void)
{
    is_cw = false;
}

struct knobState knob_states[] = {
    { // stage 0
        .A = 
        { 
            .rising = {&knob_states[0], NULL},
            .falling = {&knob_states[1], set_cw},
        },
        .B = 
        { 
            .rising = {&knob_states[0], NULL}, 
            .falling = {&knob_states[3], set_ccw}, 
        },
    },

    { // stage 1
        .A = 
        { 
            .rising = {&knob_states[0], check_ccw_turn},
            .falling = {&knob_states[1], NULL},  
        },
        .B = 
        { 
            .rising = {&knob_states[1], NULL}, 
            .falling = {&knob_states[2], NULL}, 
        },
    },
    { // stage 2
        .A = 
        { 
            .rising = {&knob_states[3], NULL},
            .falling = {&knob_states[2], NULL},
        },
        .B = 
        { 
            .rising = {&knob_states[1], NULL}, 
            .falling = {&knob_states[2], NULL}, 
        },
    },
    { // stage 3
        .A = 
        { 
            .rising = {&knob_states[3], NULL},
            .falling = {&knob_states[2], NULL},
        },
        .B = 
        { 
            .rising = {&knob_states[0], check_cw_turn}, 
            .falling = {&knob_states[3], NULL},
        },
    }
};

static void pressed(void) {
    assert(isInitialized);
    if (!lock_push) {
        fell = true;
    }
} 

static void unpressed(void) {
    assert(isInitialized);
    if (fell) {
        lock_push = true;
        timeout_start_timer(&lock_push);
        fell = false;
        push_counter++;
        if (push_counter == 3) {
            push_counter = 0;
        }
        notifyPushSubscribers();
    }
} 

struct pushState rotary_push_states[] = {
    {
        .rising = {&rotary_push_states[0], NULL},
        .falling = {&rotary_push_states[1], pressed},
    },
    {
        .rising = {&rotary_push_states[0], unpressed},
        .falling = {&rotary_push_states[1], NULL},
    }
};
/*
    END STATEMACHINE
*/

struct knobState* pCurrentKnobState = &knob_states[0];
struct pushState* pCurrentPushState = &rotary_push_states[0];

void rotary_addPushSubscriber(void (*callback)(int push_count)) {
    assert(isInitialized);
    if (push_sub_count < MAX_SUBSCRIBERS) {
        pushSubscribers[push_sub_count] = callback;
        push_sub_count++;
    }
}

void rotary_addKnobSubscriber(void (*callback)(int turn_counter)) {
    assert(isInitialized);
    if (knob_sub_count < MAX_SUBSCRIBERS) {
        knobSubscribers[knob_sub_count] = callback;
        knob_sub_count++;
    }
}

void rotary_processknobState(int chip, int pin, bool isRising) {
    assert(isInitialized);
    if (chip == GPIO_CHIP_2) {
        struct stateKnobEvent* pStateKnobEvent = NULL;
        if(pin == PIN_A) {
            if (isRising) {
                pStateKnobEvent = &pCurrentKnobState->A.rising;
            } 
            else {
                pStateKnobEvent = &pCurrentKnobState->A.falling;
            } 
        }
        else if(pin == PIN_B){
            if (isRising) {
                pStateKnobEvent = &pCurrentKnobState->B.rising;
            } 
            else {
                pStateKnobEvent = &pCurrentKnobState->B.falling;
            } 
        }
        // Do the action
        if (pStateKnobEvent->action != NULL) {
            pStateKnobEvent->action();
        }
        pCurrentKnobState = pStateKnobEvent->pNextState;
    }
}

void rotary_processPushState(int chip, int pin, bool isRising) {
    assert(isInitialized);
    if (chip == GPIO_CHIP_0 && pin == 10) {
        struct statePushEvent* pStatePushEvent = NULL;
        if (isRising) {
            pStatePushEvent = &pCurrentPushState->rising;
        } 
        else {
            pStatePushEvent = &pCurrentPushState->falling;
        }
        // Do the action
        if (pStatePushEvent->action != NULL) {
            pStatePushEvent->action();
        }
        pCurrentPushState = pStatePushEvent->pNextState;
    }
}

int rotary_getKnobCounter(void) {
    assert(isInitialized);
    return knob_counter;
}

int rotary_getPushCounter(void) {
    assert(isInitialized);
    return push_counter;
}

void rotary_init(void) {
    assert(!isInitialized);
    isInitialized = true;
    Gpio_addLineToBulk(GPIO_CHIP_2, PIN_A, rotary_processknobState);
    Gpio_addLineToBulk(GPIO_CHIP_2, PIN_B, rotary_processknobState);
    Gpio_addLineToBulk(GPIO_CHIP_0, PUSH_PIN, rotary_processPushState);
}