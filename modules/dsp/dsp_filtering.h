/*
 * dsp_filtering.h
 *
 *  Created on: Mar 7, 2026
 *      Author: crisemble
 */

#ifndef MODULES_DSP_DSP_FILTERING_H_
#define MODULES_DSP_DSP_FILTERING_H_

#include <stdint.h>

#include "arm_math.h"
#include "prism_utils.h"

// A silence filter to prevent the visualizer from detecting background humming and buzzing
#define NOISE_THRESHOLD 10 // We will probably change this later

#define GRAVITY_SHIFT 4 // This is the temporal filter speed decay for smooth falling bars/waves, etc

void ApplyDSPFilters(q15_t* current_magnitudes, uint16_t size);


#endif /* MODULES_DSP_DSP_FILTERING_H_ */
