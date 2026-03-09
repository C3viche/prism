/*
 * binning.c
 *
 *  Created on: Mar 9, 2026
 *      Author: crisemble
 */

#include "binning.h"

static uint16_t bin_cutoffs[MAX_POSSIBLE_BARS + 1] = {0};
static uint8_t initialized_bars = 0;

// Calculate an array of bin cutoff indexes
static void
CalculateBinCutoffs(uint8_t num_bars) {

    // Prevent dividing by zero or going out of bounds
    if (num_bars == 0 || num_bars > MAX_POSSIBLE_BARS) {
        printf("Error: Number of bars specified is out of bounds.\n");
        return;
    }

    // Calculate the total logarithmic range ratio
    float ratio = MAX_BIN_INDEX / MIN_BIN_INDEX;

    // Generate the cutoffs and space them logarithmically
    int i;
    for (i = 0; i <= num_bars; i++) {
        // Calculate the exact float value
        float exact_cutoff = MIN_BIN_INDEX * powf(ratio, (float)i / (float)num_bars);

        // Round it to the nearest integer index and store it
        bin_cutoffs[i] = (uint16_t)roundf(exact_cutoff);
    }

    // Save this so we know we've already done the math for this size
    initialized_bars = num_bars;

}

// Populate the `bin_peaks` array
void
BinPeaks(q15_t* frequency_magnitudes, uint8_t num_bars, q15_t* bin_peaks) {

    // Calculate the cutoffs
    // Only run the heavy floating-point math if the size changed (or on first boot)
    if (num_bars != initialized_bars) {
        CalculateBinCutoffs(num_bars);
    }

    // Loop through all the cutoffs taking min and max indexes for each bar
    uint8_t bar_idx;
    for (bar_idx = 0; bar_idx < num_bars; bar_idx++) {
        uint16_t min = bin_cutoffs[bar_idx];
        uint16_t max = bin_cutoffs[bar_idx + 1];

        // Reset the peak for THIS specific frame so it doesn't get stuck
        bin_peaks[bar_idx] = 0;

        // Get the peak of each subsection
        uint16_t i;
        for(i = min; i < max; i++) {
            if (frequency_magnitudes[i] > bin_peaks[bar_idx]) {
                bin_peaks[bar_idx] = frequency_magnitudes[i]; // new max
            }
        }

    }
}



//for (int bar = 0; bar < current_num_bars; bar++) {
//
//    // 1. BASS (Roughly the first 30% of your bars)
//    if (bar < (current_num_bars * 0.3)) {
//        draw_led_column(bar, height, COLOR_RED);
//    }
//    // 2. MIDS (The middle 40% of your bars)
//    else if (bar < (current_num_bars * 0.7)) {
//        draw_led_column(bar, height, COLOR_GREEN);
//    }
//    // 3. TREBLE (The last 30% of your bars)
//    else {
//        draw_led_column(bar, height, COLOR_BLUE);
//    }
//}



