/*
 * dsp_filtering.c
 *
 *  Created on: Mar 7, 2026
 *      Author: crisemble
 */

#include "dsp_filtering.h"

static q15_t previous_magnitudes[FFT_SIZE] = {0};

void
ApplyDSPFilters(q15_t* current_magnitudes, uint16_t size, uint8_t gravity_shift) {
    // DC Voltage blocking
    current_magnitudes[0] = 0;

    uint16_t i;
    for(i = 1; i < size; i++) {
        // Taper the threshold down as the frequency gets higher
        int16_t dynamic_threshold = NOISE_THRESHOLD - (i / 4);

        // Don't let noise threshold get too low
        if (dynamic_threshold < 3) dynamic_threshold = 3;

        // Silence filter to prevent background humming/buzzing to affect our visualizer
        if(current_magnitudes[i] < dynamic_threshold) {
            current_magnitudes[i] = 0;// Silence if below the threshold
        }

        // Gravity filter
        // Magnitude = (Speed * New_FFT_Value) + ((1 - Speed) * Previous_LED_Height)
        if (current_magnitudes[i] > previous_magnitudes[i]) {
            previous_magnitudes[i] = current_magnitudes[i];
        }
        // If the new beat is quieter, slowly let the old bar fall down (Gravity)
        else {
            uint8_t dynamic_shift = gravity_shift; // Base is 4 (falls by 1/16th)

            if (i > (BASS * size)) dynamic_shift = gravity_shift + 1; // Mids fall by 1/32nd
            if (i > (MID * size)) dynamic_shift = gravity_shift + 2; // Treble falls by 1/64th

            // Calculate decay step
            int16_t decay = (previous_magnitudes[i] >> dynamic_shift);

            // If the bar is too small to decay via shifting, force it down
            if (decay == 0 && previous_magnitudes[i] > 0) {
                    decay = 1;
            }

            // Subtract a fraction of its own height to make it fall.
            previous_magnitudes[i] = previous_magnitudes[i] - decay;

            // Prevent tiny "ghost" values from showing up on OLED
            if (previous_magnitudes[i] < 2) {
                previous_magnitudes[i] = 0;
            }

            // Overwrite the current frame with the smoothed falling value
            current_magnitudes[i] = previous_magnitudes[i];
        }


    }


}
