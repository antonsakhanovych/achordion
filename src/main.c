#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define BUFFER_SIZE_FRAMES 2048

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    // Make sure the format is ma_format_f32
    if(pDevice->capture.format != ma_format_f32) {
        fprintf(stderr, "ERROR: Format should be ma_format_f32\n");
        exit(1);
    }
    // Acquire the ring buffer
    ma_pcm_rb* ring_buf = (ma_pcm_rb*)pDevice->pUserData;
    // Writeable buffer that will be available after calling ma_pcm_rb_acquire_write
    float* buffer_to_write;
    // NOTE: frameCount is updated to the amount of frames allowed to write
    ma_pcm_rb_acquire_write(ring_buf, &frameCount, (void**)&buffer_to_write);
    // Here write to the buffer
    memcpy(buffer_to_write, pInput, sizeof(float) * frameCount);
    ma_pcm_rb_commit_write(ring_buf, frameCount);
}

int main(int argc, char** argv)
{
    ma_pcm_rb ring_buf;
    ma_device_config device_config = ma_device_config_init(ma_device_type_capture);
    ma_result result;
    ma_device device;
    device_config.capture.format = ma_format_f32;
    device_config.capture.channels = NUM_CHANNELS;
    device_config.sampleRate = SAMPLE_RATE;
    device_config.dataCallback = data_callback;
    device_config.pUserData = &ring_buf;

    result = ma_pcm_rb_init(ma_format_f32, NUM_CHANNELS, BUFFER_SIZE_FRAMES, NULL, NULL, &ring_buf);
    if(result != MA_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize ring buffer.");
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
//#define MINIAUDIO_IMPLEMENTATION
//#include "miniaudio.h"
//
//#include <stdio.h>
//
//int main(int argc, char** argv)
//{
//    ma_result result;
//    ma_context context;
//    ma_device_info* pPlaybackDeviceInfos;
//    ma_uint32 playbackDeviceCount;
//    ma_device_info* pCaptureDeviceInfos;
//    ma_uint32 captureDeviceCount;
//    ma_uint32 iDevice;
//
//    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
//        printf("Failed to initialize context.\n");
//        return -2;
//    }
//
//    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
//    if (result != MA_SUCCESS) {
//        printf("Failed to retrieve device information.\n");
//        return -3;
//    }
//
//    printf("Playback Devices\n");
//    for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
//        printf("    %u: %s\n", iDevice, pPlaybackDeviceInfos[iDevice].name);
//    }
//
//    printf("\n");
//
//    printf("Capture Devices\n");
//    for (iDevice = 0; iDevice < captureDeviceCount; ++iDevice) {
//        printf("    %u: %s\n", iDevice, pCaptureDeviceInfos[iDevice].name);
//    }
//
//
//    ma_context_uninit(&context);
//
//    (void)argc;
//    (void)argv;
//    return 0;
//}
//
