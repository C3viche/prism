/*
 * fft.h
 *
 *  Created on: Mar 5, 2026
 *      Author: crisemble
 */

#ifndef MODULES_FFT_FFT_H_
#define MODULES_FFT_FFT_H_

#include "arm_math.h"
#include "prism_utils.h"

// 512 sample audio buffer (can be modified later)

#define MAX_MAGNITUDE   200

// Scale factor to increase magnitudes to a height that makes sense for the oled pixels
#define OLED_SCALE 2
#define OLED_DIM   128

// Declare the FFT Instance structure
static arm_rfft_instance_q15 fft_instance;

// Allocate the arrays (q15_t is just a standard int16_t)
static q15_t fft_output[FFT_SIZE * 2];          // FFT output (Real + Imaginary pairs, that's why it's double)

void InitFFT(void);
void ProcessAudioFrame(q15_t audio_input[FFT_SIZE], q15_t frequency_magnitudes[FFT_SIZE/2]);
static void ScaleMagnitudes(q15_t frequency_magnitudes[FFT_SIZE/2]);

#endif /* MODULES_FFT_FFT_H_ */
