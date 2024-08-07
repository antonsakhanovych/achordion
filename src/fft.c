#include "fft.h"
#include <math.h>
#include <stdlib.h>


// https://cp-algorithms.com/algebra/fft.html
void fft(const float* samples, size_t size, complex float* output)
{
    if(size == 1) {
        output[0] = samples[0];
        return;
    }
    float vec_even[size/2];
    float vec_odd[size/2];
    complex float even_out[size/2];
    complex float odd_out[size/2];

    for(size_t i = 0; i * 2 < size; ++i) {
        vec_even[i] = samples[2 * i];
        vec_odd[i] = samples[2 * i + 1];
    }
    fft(vec_even, size/2, even_out);
    fft(vec_odd,size/2, odd_out);
    float ang = 2 * -M_PI / size;
    complex float w = 1;
    complex float wn = cosf(ang) + I * sinf(ang);
    for(size_t i = 0; 2 * i < size; ++i) {
        output[i] = even_out[i] + w * odd_out[i];
        output[i + size/2] = even_out[i] - w * odd_out[i];
        w *= wn;
    }
}
