/*
 * binning.h
 *
 *  Created on: Mar 9, 2026
 *      Author: crisemble
 */

#ifndef MODULES_BINNING_BINNING_H_
#define MODULES_BINNING_BINNING_H_

#include <math.h>
#include <stdint.h>
#include "prism_utils.h"
#include "arm_math.h"

#define BIN_WIDTH (ADC_SAMPLE_RATE / FFT_SIZE) // HZ per bin

// Min and max bars to be displayed on the screen
#define MAX_POSSIBLE_BARS 64
#define MIN_POSSIBLE_BARS 3

// Automatically calculates to 255.0f for a 512-point FFT
// Formula: (FFT_SIZE / 2) - 1
#define MAX_BIN_INDEX ((float)(FFT_SIZE / 2) - 1.0f)
#define MIN_BIN_INDEX 1.0f // skips 0 Hz noise

// Shared functions
void BinPeaks(q15_t frequency_magnitudes[FFT_SIZE / 2], uint8_t num_bars, q15_t* bin_peaks);

#endif /* MODULES_BINNING_BINNING_H_ */
