#include "audio_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

static int init = 0;
static pthread_t thread;

static void* audioProcess(){
    assert(init);
    while(init){
        system("ffmpeg -hide_banner -loglevel error -ar 44100 -f alsa -i default:CARD=U0x46d0x825 -t 10 -acodec mp3 -f mp3 udp://192.168.7.2:12343");
    }
}

void AudioProc_init(){
    init = 1;
    pthread_create(&thread, NULL, audioProcess, NULL);
}

void AudioProc_cleanup(){
    init = 0;
    pthread_join(thread, NULL);
}