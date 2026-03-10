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

void InitFFT(void);
void ProcessAudioFrame(q15_t audio_input[FFT_SIZE], q15_t frequency_magnitudes[FFT_SIZE/2]);

#endif /* MODULES_FFT_FFT_H_ */
