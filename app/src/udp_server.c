#include "udp_server.h"
#include "hal/panTilt.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_LEN 1024
#define PORT 12345

static bool isInitialized = false;
static bool *main_thread_stop_var;
static pthread_t serverThread;

static void process_response(char *messageRx, char *messageTx) {
    char *command = strtok(messageRx, " \n");

    if (!command) {
        return;
    }
    
    char *val_str = strtok(NULL, " \n");
    int val = -1;
    if (val_str) {
        val = strcmp(val_str, "null") == 0 ? -1 : atoi(val_str);
    }

    if (strcmp(command, "stop") == 0) {
        snprintf(messageTx, MAX_LEN, "User requested to quit\n");
        *main_thread_stop_var = true;
        return;
    }
    if (strcmp(command, "pan") == 0) {
        panTilt_setPercent(PAN, 100 * val);
        snprintf(messageTx, MAX_LEN, "pan %d\n", 100 * val);
        return;
    }
    if (strcmp(command, "tilt") == 0) {
        panTilt_setPercent(TILT, 100 * val);
        snprintf(messageTx, MAX_LEN, "tilt %d\n", 100 * val);
        return;
    }
    if (strcmp(command, "zoom") == 0) {
        // TODO
        return;
    }
    if (strcmp(command, "talk") == 0) {
        // TODO
        return;
    }
}

void* UDPServer_Thread(void* args) {
    (void) args;
    
    assert(isInitialized);

    printf("UDP server started\n");
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    // open socket
    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
    
    while (!*main_thread_stop_var) {
        // receive data
        struct sockaddr_in sinRemote;
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MAX_LEN];
        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, (struct sockaddr *) &sinRemote, &sin_len);
        // Null terminated (string):
        messageRx[bytesRx] = 0;

        // create reply
        char messageTx[MAX_LEN];

        process_response(messageRx, messageTx);

        sin_len = sizeof(sinRemote);
        sendto(socketDescriptor, messageTx, strlen(messageTx), 0, (struct sockaddr *) &sinRemote, sin_len);
    }

    // close socket
    close(socketDescriptor);

    pthread_exit(NULL);
}
void UDPServer_init(bool* stop_var) {
    assert(!isInitialized);
    isInitialized = true;

    main_thread_stop_var = stop_var;

    if (pthread_create(&serverThread, NULL, &UDPServer_Thread, NULL) != 0) {
        perror("Unable to initialize UDP Server thread");
        exit(EXIT_FAILURE);
    }
}
void UDPServer_cleanup() {
    pthread_join(serverThread, NULL);
    assert(isInitialized);
    isInitialized = false;
}
