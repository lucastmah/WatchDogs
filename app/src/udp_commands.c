#include "udp_commands.h"
#include "camera_controls.h"
#include "nightLight.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LEN 1500
#define PORT 12345
#define SAMPLE_BUF_LEN 10

static _Atomic bool open_port = false;
static pthread_t thread_id;
static struct timespec startTime;

static char help[] = "\n\
Accepted command examples:\n\
zoom        -- changes camera zoom. 0 to zoom in, 1 to zoom out.\n\
pan         -- moves camera horizontally. -1 for left, 1 for right.\n\
tilt        -- moves camera vertically. -1 for down, 1 for up.\n\
patrol      -- sets camera patrol mode, 1 for panning left and right, 0 to stop.\n\
mute        -- toggles the sound. 0 for unmute, 1 for mute.\n\
talk        -- toggles the mic. 0 to disable mic, 1 to enable mic.\n\
stop        -- shuts down BeagleY-AI program.\n\
<enter>     -- repeat last command.\n";

static char* commands[] = {"zoom", "pan", "tilt", "patrol", "mute", "talk", "motion_light", "stop", "help", "?", 0};

static bool* main_thread_stop_ptr;

static int retrieve_num_from_string(char* input) {
    char* endptr;
    int num = strtol(input, &endptr, 10);
    if (endptr == input) {
        return -1;
    }
    if (*endptr != '\0') {
        return -1;
    }
    return num;
}

static void reply_command(int command, char* param, int socketDescriptor, struct sockaddr_in sinRemote) {
    char messageTx[MAX_LEN];
    unsigned int sin_len = sizeof(sinRemote);
    int int_param;
    if (param != NULL) {
        int_param = retrieve_num_from_string(param);
    }
    switch (command) {
        case 0:
            snprintf(messageTx, MAX_LEN, "zoom %d\n", int_param);
            break;
        case 1:
            snprintf(messageTx, MAX_LEN, "pan %d\n", int_param);
            CameraControls_pan(int_param);
            break;
        case 2:
            snprintf(messageTx, MAX_LEN, "tilt %d\n", int_param);
            CameraControls_tilt(int_param);
            break;
        case 3:
            bool is_patrolling;
            if (int_param != -1) {
                is_patrolling = CameraControls_setPatrolMode(int_param);
            } else {
                is_patrolling = CameraControls_getPatrolMode();
            }
            snprintf(messageTx, MAX_LEN, "%d\n", is_patrolling);
            break;
        case 4:
            snprintf(messageTx, MAX_LEN, "mute %d\n", int_param);
            break;
        case 5:
            snprintf(messageTx, MAX_LEN, "talk %d\n", int_param);
            break;
        case 6:
            bool nightlight;
            if (int_param != -1) {
                nightlight = nightLight_setLightMode(int_param);
            } else {
                nightlight = nightLight_getLightMode();
            }
            snprintf(messageTx, MAX_LEN, "%d\n", nightlight);
            break;
        case 7:
            snprintf(messageTx, MAX_LEN, "stopping...\n");
            open_port = false;
            break;
        case 8:
        case 9:
            snprintf(messageTx, MAX_LEN, help, int_param);
            break;
        default:
            snprintf(messageTx, MAX_LEN, "Unknown command. Type 'Help' for list of valid commands.\n");
            break;
    }
    sendto(socketDescriptor, messageTx, strlen(messageTx), 0, (struct sockaddr*) &sinRemote, sin_len);
}

static void* listen_to_port() {
    assert(open_port);
    int command = -1;
    // Start Socket
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
    
    while (open_port) {
        // Receive Data
        struct sockaddr_in sinRemote;
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MAX_LEN];

        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, (struct sockaddr*) &sinRemote, &sin_len);
        messageRx[bytesRx] = 0;

        // parse input into command and parameters
        char* saveptr;
        char* ptr = strtok_r(messageRx, " ", &saveptr);
        
        command = -1;
        for (int i = 0; commands[i]; i++) {
            if (strncmp(ptr, commands[i], strlen(commands[i])) == 0) {
                command = i;
            }
        }

        ptr = strtok_r(NULL, " \n", &saveptr);

        // Reply
        reply_command(command, ptr, socketDescriptor, sinRemote);
    }
    // Stop main thread
    *main_thread_stop_ptr = true;

    // Close socket
    close(socketDescriptor);
    return NULL;
}

void UDPCommands_init(bool *stop_var_ptr) {
    assert(!open_port);
    open_port = true;
    main_thread_stop_ptr = stop_var_ptr;
    clock_gettime(CLOCK_REALTIME, &startTime);
    pthread_create(&thread_id, NULL, listen_to_port, NULL);
}

void UDPCommands_cleanup() {
    pthread_join(thread_id, NULL);
}