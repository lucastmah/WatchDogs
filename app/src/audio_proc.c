#include "audio_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <subprocess.h>

static int pid_t process;

void AudioProc_init(){
    process = fork();
    assert(process >= 0);
    if(!process){
        system("ffmpeg -hide_banner -loglevel error -ar 44100 -f alsa -i default:CARD=C920 -acodec mp3 -f mp3 udp://192.168.7.1:12343");
    }
}

void AudioProc_cleanup(){
    kill(process, SIGKILL);
}