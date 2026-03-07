/*
 * fft.c
 *
 *  Created on: Mar 5, 2026
 *      Author: crisemble
 */

#include "fft.h"


// Initialize the FFT instance.
void
InitFFT(void) {
    // Parameters: &instance, length, 0 (Forward FFT, not Inverse), 1 (Bit Reversal ON)
    arm_status status = arm_rfft_init_q15(&fft_instance, FFT_SIZE, 0, 1);

    if (status != ARM_MATH_SUCCESS) {
        // Handle initialization error (usually means an unsupported FFT_SIZE)
        while(1);
    }

}

void
ProcessAudioFrame(void) {
    // Prepare the microphone data for FFT to silence

    // Remove the DC Offset. If your ADC idles at 2048 (half of a 12-bit range),
    // subtract 2048 from every sample. For now we will just set to 0
    int i;
    for (i = 0; i < FFT_SIZE; i++) {
        audio_input[i] = 0; // silence = 0
    }

    // Run FFT algorithm from CMSIS_DSP
    // This converts the time-domain audio wave into frequency-domain data
    arm_rfft_q15(&fft_instance, audio_input, fft_output);

    // Calculate magnitude
    // This takes the confusing Real/Imaginary pairs and calculates the absolute volume
    // of each frequency bin. We only calculate FFT_SIZE/2 because the second half
    // of a Real FFT is just a mirror image of the first half.
    arm_cmplx_mag_q15(fft_output, frequency_magnitudes, FFT_SIZE / 2);

    // TODO: recalculate ranges based on ADC sampling rate
    // frequency_magnitudes[0] is 0Hz (DC) noise. Ignore it.
    // frequency_magnitudes[1] up to ~10 might be heavy bass frequencies.
    // frequency_magnitudes[200+] could be high treble.

//    update_oled_and_leds(frequency_magnitudes);
}


