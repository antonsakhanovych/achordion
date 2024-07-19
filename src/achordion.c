#include <errno.h>
#include <pthread.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define BUFFER_SIZE_FRAMES 2048

typedef struct {
    ma_pcm_rb ring_buffer;
    pthread_mutex_t mutex;
} AudioData;

int audio_data_init(AudioData* audio_data);

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    // Make sure the format is ma_format_f32
    if(pDevice->capture.format != ma_format_f32) {
        fprintf(stderr, "ERROR: Format should be ma_format_f32\n");
        exit(1);
    }
    // Acquire the AudioData struct
    AudioData* audio_data = (AudioData *)pDevice->pUserData;
    // Writeable buffer that will be available after calling ma_pcm_rb_acquire_write
    float* buffer_to_write;
    // Acquire mutex lock
    if(pthread_mutex_lock(&audio_data->mutex) == EOWNERDEAD) {
        fprintf(stderr, "ERROR: Owner thread that locked the mutex is dead\n");
        return;
    }
    // NOTE: frameCount is updated to the amount of frames allowed to write
    ma_pcm_rb_acquire_write(&audio_data->ring_buffer, &frameCount, (void**)&buffer_to_write);
    // Here write to the buffer
    memcpy(buffer_to_write, pInput, sizeof(float) * frameCount);
    // Commit the changes to the ring buffer
    ma_pcm_rb_commit_write(&audio_data->ring_buffer, frameCount);
    // Unlock the mutex
    pthread_mutex_unlock(&audio_data->mutex);
}

int main(int argc, char** argv)
{
    ma_result result;
    AudioData audio_data = {0};
    ma_device_config device_config = ma_device_config_init(ma_device_type_capture);
    ma_device device;
    device_config.capture.format = ma_format_f32;
    device_config.capture.channels = NUM_CHANNELS;
    device_config.sampleRate = SAMPLE_RATE;
    device_config.dataCallback = data_callback;
    device_config.pUserData = &audio_data;

    if(audio_data_init(&audio_data) < 0) {
        fprintf(stderr, "ERROR: Failed to initialize audio data\n");
        return -1;
    }

    result = ma_device_init(NULL, &device_config, &device);
    if(result != MA_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize the device.");
        return -1;
    }
    result = ma_device_start(&device);
    if(result != MA_SUCCESS) {
        ma_device_uninit(&device);
        fprintf(stderr, "ERROR: Failed to start the device.");
        return -1;
    }

    printf("INFO\tChannels: %d\n", device.capture.channels);
    printf("INFO\tSampleRate: %d\n", device.sampleRate);

    int width = 1200;
    int height = 900;
    InitWindow(width, height, "achordion");

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();

    ma_device_uninit(&device);

    return 0;
}

int audio_data_init(AudioData* audio_data)
{
    if(pthread_mutex_init(&audio_data->mutex, NULL) < 0) {
        fprintf(stderr, "ERROR: Failed to initialize mutex");
        return -1;
    }

    ma_result result = ma_pcm_rb_init(ma_format_f32,
                                      NUM_CHANNELS,
                                      BUFFER_SIZE_FRAMES,
                                      NULL,
                                      NULL,
                                      &audio_data->ring_buffer);
    if(result != MA_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize ring buffer.");
        return -1;
    }
    return 0;
}
