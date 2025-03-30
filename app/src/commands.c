#include "commands.h"
#include <stdbool.h>
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
pan         -- moves camera horizontally. 0 for left, 1 for right.\n\
tilt        -- moves camera vertically. 0 for down, 1 for up.\n\
mute        -- toggles the sound. 0 for unmute, 1 for mute.\n\
talk        -- toggles the mic. 0 to disable mic, 1 to enable mic.\n\
stop        -- shuts down BeagleY-AI program.\n\
<enter>     -- repeat last command.\n";

static char* commands[] = {"zoom", "pan", "tilt", "mute", "talk", "stop", "help", "?", 0};

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
            break;
        case 2:
            snprintf(messageTx, MAX_LEN, "tilt %d\n", int_param);
            break;
        case 3:
            snprintf(messageTx, MAX_LEN, "mute %d\n", int_param);
            break;
        case 4:
            snprintf(messageTx, MAX_LEN, "talk %d\n", int_param);
            break;
        case 5:
            snprintf(messageTx, MAX_LEN, "stopping %d\n", int_param);
            open_port = false;
            break;
        case 6:
        case 7:
            snprintf(messageTx, MAX_LEN, help, int_param);
            break;
        default:
            snprintf(messageTx, MAX_LEN, "Unknown command. Type 'Help' for list of valid commands.\n");
            break;
    }
    printf("%s\n", messageTx);
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

    // Close socket
    close(socketDescriptor);
    return NULL;
}

void commands_init() {
    assert(!open_port);
    open_port = true;
    clock_gettime(CLOCK_REALTIME, &startTime);
    pthread_create(&thread_id, NULL, listen_to_port, NULL);
}

void commands_cleanup() {
    pthread_join(thread_id, NULL);
}