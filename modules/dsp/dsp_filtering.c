/*
 * dsp_filtering.c
 *
 *  Created on: Mar 7, 2026
 *      Author: crisemble
 */

#include "dsp_filtering.h"

static q15_t previous_magnitudes[FFT_SIZE] = {0};

void
ApplyDSPFilters(q15_t* current_magnitudes, uint16_t size) {
    // DC Voltage blocking
    current_magnitudes[0] = 0;

    uint16_t i;
    for(i = 1; i < size; i++) {
        // Silence filter to prevent background humming/buzzing to affect our visualizer
        if(current_magnitudes[i] < NOISE_THRESHOLD) {
            current_magnitudes[i] = 0;// Silence if below the threshold
        }

        // Gravity filter
        // Magnitude = (Speed * New_FFT_Value) + ((1 - Speed) * Previous_LED_Height)
        if (current_magnitudes[i] > previous_magnitudes[i]) {
            previous_magnitudes[i] = current_magnitudes[i];
        }
        // If the new beat is quieter, slowly let the old bar fall down (Gravity)
        else {
            // Subtract a fraction of its own height to make it fall.
            previous_magnitudes[i] = previous_magnitudes[i] - (previous_magnitudes[i] >> 2);

            // Overwrite the current frame with the smoothed falling value
            current_magnitudes[i] = previous_magnitudes[i];
        }


    }


}




