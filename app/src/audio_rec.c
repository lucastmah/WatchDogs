#include "audio_rec.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>


#define PORT_T 12343
static struct sockaddr_in sinT;

static pthread_t thread_id;

static const int stereo_channels = 2;
static const int buffer_frames = 128;
static const unsigned int rate = 44100;
static snd_pcm_t *capture_handle;
static snd_pcm_hw_params_t *hw_params;
static snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

static int init = 0;

// Initialize UDP connection
static void openConnectionT(void)
{
    memset(&sinT, 0, sizeof(sinT));
    sinT.sin_family = AF_INET;
    sinT.sin_addr.s_addr = htonl(INADDR_ANY);
    sinT.sin_port = htons(PORT_T);

    socketDescriptorT = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptorT, (struct sockaddr *)&sinT, sizeof(sinT));

    // sinRemoteT.sin_family = AF_INET;
    // sinRemoteT.sin_port = htons(RPORT_T);
    // sinRemoteT.sin_addr.s_addr = inet_addr("192.168.7.2");
}

static void* audioCapture(){
    assert(init);
    char *buffer;
    
    buffer = malloc(buffer_frames * snd_pcm_format_width(format) / 8 * stereo_channels);

    fprintf(stdout, "buffer allocated\n");
    while(init){
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            fprintf (stderr, "read from audio interface failed (%s)\n",
                        snd_strerror (err));
            exit (1);
        }
        fprintf(stdout, "read %d done\n", i);
    }
    
    free(buffer);
    fprintf(stdout, "buffer freed\n");

    return NULL;
}

void AudioRec_init(){
    int err;

    if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n", 
                    argv[1],
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "audio interface opened\n");
            
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params allocated\n");
                    
    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params initialized\n");
    
    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params access setted\n");
    
    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params format setted\n");
    
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                    snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params rate setted\n");

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params channels setted\n");
    
    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params setted\n");
    
    snd_pcm_hw_params_free (hw_params);

    fprintf(stdout, "hw_params freed\n");
    
    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                    snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "audio interface prepared\n");
    init = 1;

    pthread_create(&thread_id, NULL, audioCapture, NULL);
} 

void AudioRec_cleanup(){
    init = 0;

    pthread_join(thread_id, NULL);
    
    snd_pcm_close (capture_handle);
    fprintf(stdout, "audio interface closed\n");
}