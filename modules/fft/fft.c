/*
 * fft.c
 *
 *  Created on: Mar 5, 2026
 *      Author: crisemble
 */

#include "fft.h"

#include "dsp/dsp_filtering.h"

// Declare the FFT Instance structure
static arm_rfft_instance_q15 fft_instance;

// Allocate the arrays (q15_t is just a standard int16_t)
static q15_t fft_output[FFT_SIZE * 2];          // FFT output (Real + Imaginary pairs, that's why it's double)

// Initialize the FFT instance.
void
InitFFT(void) {
    // Parameters: &instance, length, 0 (Forward FFT, not Inverse), 1 (Bit Reversal ON)
    arm_status status = arm_rfft_init_q15(&fft_instance, FFT_SIZE, 0, 1);

    if (status != ARM_MATH_SUCCESS) {
        // Handle initialization error (usually means an unsupported FFT_SIZE)
        printf("Initialization error. FFT_SIZE may be unsupported.");
        while(1);
    }

}

// Scale the magnitudes to a reasonable "height" for the oled
static void
ScaleMagnitudes(q15_t frequency_magnitudes[FFT_SIZE/2]) {
    int i;
    for(i = 1; i < FFT_SIZE / 2; i++) {
        // Use a 32-bit integer for the dangerous magnitude multiplication
        int32_t raw_mag = frequency_magnitudes[i];
        int32_t scaled_mag = raw_mag * OLED_SCALE;

        // Cap it BEFORE it has a chance to overflow the final calculation
        if (scaled_mag > MAX_MAGNITUDE) {
            scaled_mag = MAX_MAGNITUDE;
        }

        // Normalize to OLED dimensions
        scaled_mag = (scaled_mag * OLED_DIM) / MAX_MAGNITUDE;

        frequency_magnitudes[i] = (q15_t)scaled_mag;
    }
}

void
ProcessAudioFrame(q15_t audio_input[FFT_SIZE], q15_t frequency_magnitudes[FFT_SIZE/2]) {
    // Prepare the microphone data for FFT to silence

    // Remove the DC Offset. If your ADC idles at 2048 (half of a 12-bit range),
    // subtract 2048 from every sample.
    int i;
    for (i = 0; i < FFT_SIZE; i++) {
        audio_input[i] = audio_input[i] - ADC_DC_OFFSET; // silence = ADC_DC_OFFSET (2048)
    }

    // Run FFT algorithm from CMSIS_DSP
    // This converts the time-domain audio wave into frequency-domain data
    arm_rfft_q15(&fft_instance, audio_input, fft_output);

    // Calculate magnitude
    // This takes the confusing Real/Imaginary pairs and calculates the absolute volume
    // of each frequency bin. We only calculate FFT_SIZE/2 because the second half
    // of a Real FFT is just a mirror image of the first half.
    arm_cmplx_mag_q15(fft_output, frequency_magnitudes, FFT_SIZE / 2);

    // Calculate ranges based on ADC sampling rate (probably 16 kHz to 20 kHz)
    // frequency_magnitudes[0] is 0Hz (DC) noise. Ignore it.
    // frequency_magnitudes[1] up to ~10 might be heavy bass frequencies.
    // frequency_magnitudes[200+] could be high treble.

    // Apply the DSP filtering to smoothen out sound ranges
    ApplyDSPFilters(frequency_magnitudes, FFT_SIZE / 2);

    // TODO: Find accurate maximum for magnitudes --> and make it a proportion of OLED_DIM
    ScaleMagnitudes(frequency_magnitudes);

}


