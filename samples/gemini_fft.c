#include "arm_math.h"

#define FFT_SIZE 512

// 1. Declare the FFT Instance structure
arm_rfft_instance_q15 fft_instance;

// 2. Allocate the arrays (q15_t is just a standard int16_t)
q15_t audio_input[FFT_SIZE];             // Your raw ADC microphone readings
q15_t fft_output[FFT_SIZE * 2];          // The raw FFT output (Real + Imaginary pairs)
q15_t frequency_magnitudes[FFT_SIZE/2];  // The final, usable volume levels for your display

void init_visualizer_dsp(void) {
    // Initialize the FFT instance.
    // Parameters: &instance, length, 0 (Forward FFT, not Inverse), 1 (Bit Reversal ON)
    arm_status status = arm_rfft_init_q15(&fft_instance, FFT_SIZE, 0, 1);
    
    if (status != ARM_MATH_SUCCESS) {
        // Handle initialization error (usually means an unsupported FFT_SIZE)
        while(1); 
    }
}

void process_audio_frame(void) {
    // --- STEP 1: PREP THE DATA ---
    // Remove the DC Offset. If your ADC idles at 2048 (half of a 12-bit range), 
    // subtract 2048 from every sample so silence equals 0.
    for (int i = 0; i < FFT_SIZE; i++) {
        // Example: assuming audio_input is already populated by your ADC timer/DMA
        audio_input[i] = audio_input[i] - 2048; 
    }

    // --- STEP 2: RUN THE FFT ---
    // This converts the time-domain audio wave into frequency-domain data
    arm_rfft_q15(&fft_instance, audio_input, fft_output);

    // --- STEP 3: CALCULATE MAGNITUDE ---
    // This takes the confusing Real/Imaginary pairs and calculates the absolute volume 
    // of each frequency bin. We only calculate FFT_SIZE/2 because the second half 
    // of a Real FFT is just a mirror image of the first half.
    arm_cmplx_mag_q15(fft_output, frequency_magnitudes, FFT_SIZE / 2);

    // --- STEP 4: USE THE DATA! ---
    // frequency_magnitudes[0] is your 0Hz (DC) noise. Ignore it.
    // frequency_magnitudes[1] up to ~10 are your heavy bass frequencies.
    // frequency_magnitudes[200+] are your high treble.
    
    update_oled_and_leds(frequency_magnitudes);
}
