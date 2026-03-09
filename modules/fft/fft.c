/*
 * fft.c
 *
 *  Created on: Mar 5, 2026
 *      Author: crisemble
 */

#include "fft.h"

#include "dsp/dsp_filtering.h"

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

    // Apply the dsp filtering to smoothen out sound ranges
    ApplyDSPFilters(frequency_magnitudes, FFT_SIZE);

    // TODO: Find accurate maximum for magnitudes --> and make it a proportion of OLED_DIM
    ScaleMagnitudes(frequency_magnitudes);

}

static void
ScaleMagnitudes(q15_t frequency_magnitudes[FFT_SIZE/2]) {
    int i;
    for(i = 1; i < FFT_SIZE / 2; i++) {
        q15_t mag = frequency_magnitudes[i];
        q15_t scaled_mag = mag * OLED_SCALE;

        // Scale the magnitude with a cap of the arbitrary maximum
        scaled_mag = scaled_mag < MAX_MAGNITUDE ? scaled_mag : MAX_MAGNITUDE;

        // Normalize to OLED dimensions
        scaled_mag = (scaled_mag * OLED_DIM) / MAX_MAGNITUDE;

        frequency_magnitudes[i] = scaled_mag;
    }
}


