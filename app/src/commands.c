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

/*
static char help[] = "\n\
Accepted command examples:\n\
volume      -- get volume; sets volume in range of 0 - 100 if parameter provided.\n\
mode        -- get mode; sets mode if parameter provided in range [0:2].\n\
tempo       -- get bpm; sets bpm in range of 40 - 300 bpm if parameter provided.\n\
play        -- plays a sound in range of 0 - 2 if parameter provided. \n\
read-uptime -- returns number of seconds since server has been up.\n\
stop        -- shuts down BeagleY-AI program.\n\
<enter>     -- repeat last command.\n";
*/

static char* commands[] = {"zoom", "pan", "mute", "talk", "stop", "help", "?", 0};

// static long long compute_elapsed_time(void) {
//     struct timespec curr;
//     clock_gettime(CLOCK_REALTIME, &curr);
//     return curr.tv_sec - startTime.tv_sec;
// }

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
            snprintf(messageTx, MAX_LEN, "mute %d\n", int_param);
            break;
        case 3:
            snprintf(messageTx, MAX_LEN, "talk %d\n", int_param);
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

        // convert any endlines to spaces
        // for (int i = 0; i < bytesRx; i++) {
        //     if (messageRx[i] == '\n') {
        //         messageRx[i] = ' ';
        //     }
        // }
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