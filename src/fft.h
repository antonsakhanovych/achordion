#ifndef _FFT_H
#define _FFT_H

#include <math.h>
#include <stddef.h>
#include <complex.h>

void fft(const float* samples, size_t size, complex float* output);

#endif // _FFT_H
