#include "oled.h"
#include "binning/binning.h"
#include "ir_buttons/ir_buttons.h"

#define BASE_CHAR_SCALE 6
#define BASE_LINE_WIDTH 8

extern mode_t mode;
extern void ChangeMode(char c);

// This will draw a series of bars based on the bin peaks.
static void DrawBars(size_t num_bars, q15_t* bin_peaks, uint16_t color1, uint16_t color2, uint16_t color3) {

    // Stores the Y-coordinate of the TOP of each bar from the previous frame.
    static uint8_t old_y_coords[MAX_POSSIBLE_BARS];
    static bool first_run = true;

    // On the very first frame, pretend all bars are at Y = 128 (height of 0)
    if (first_run) {
        int i;
        for(i = 0; i < MAX_POSSIBLE_BARS; i++) {
            old_y_coords[i] = OLED_DIM;
        }
        first_run = false;
    }

    // Leave a 1-pixel gap between bars so they don't blend into a single blob
    size_t bar_width = OLED_DIM / num_bars;
    size_t draw_width = bar_width - 1;

    size_t i;
    for (i = 0; i < num_bars; i++) {

        // The percentage spread by x-position
        unsigned int bar_color;
        if (i < (num_bars * BASS)) {
            bar_color = color1; // Bass
        } else if (i < (num_bars * MID)) {
            bar_color = color2; // Mid
        } else {
            bar_color = color3; // Treble
        }

        // Cap the peak just in case so it doesn't go "off screen" (shouldn't happen but better safe than sorry)
        q15_t peak = bin_peaks[i];
        if (peak > OLED_DIM) peak = OLED_DIM;

        // Calculate the physical Y-coordinates
        int new_y = OLED_DIM - peak;
        int old_y = old_y_coords[i];
        int x = i * bar_width;

        // The dirty overwrite logic to erase/update bar
        if (new_y < old_y) {
            // The bar grew TALLER. Draw the new color from the new tip down to the old tip.
            fillRect(x, new_y, draw_width, old_y - new_y, bar_color);
        } else if (new_y > old_y) {
            // The bar got SHORTER. Erase the empty space by drawing BLACK over the old tip.
            fillRect(x, old_y, draw_width, new_y - old_y, BLACK);
        }
        // NOTE: If new_y == old_y, we do literally nothing. It saves SPI bandwidth.

        // Update the memory for the next frame
        old_y_coords[i] = new_y;

    }
}

// Add the prototype so oled.c knows the function exists in Adafruit_OLED.c
extern unsigned int Color565(unsigned char r, unsigned char g, unsigned char b);

static void DrawWaves(size_t num_waves, q15_t* bin_peaks, uint16_t color1, uint16_t color2, uint16_t color3) {
    static uint8_t old_y_coords[MAX_POSSIBLE_BARS][OLED_DIM] = {0};
    int center = OLED_DIM / 2;
    int max_y = OLED_DIM - 1;

    static float phase_offset = 0.0f;
    phase_offset += PHASE_TRAVEL;
    if (phase_offset > TWO_PI) phase_offset -= TWO_PI;

    int x;
    size_t w;

    // My final attempt at making this as fast and reactive as possible
    // --- PRE-COMPUTATIONS ---
    // Calculate all the heavy math ONCE per wave, not 128 times per wave
    q15_t wave_amps[MAX_POSSIBLE_BARS];
    q15_t current_phases[MAX_POSSIBLE_BARS];
    q15_t phase_steps[MAX_POSSIBLE_BARS];

    unsigned char r_bases[MAX_POSSIBLE_BARS];
    unsigned char g_bases[MAX_POSSIBLE_BARS];
    unsigned char b_bases[MAX_POSSIBLE_BARS];

    for (w = 0; w < num_waves; w++) {
        wave_amps[w] = bin_peaks[w] / 2;

        float frequency = 0.08f + (0.01f * w);
        float wave_shift = phase_offset + (w * 1.0f);

        // Convert starting position and step size to Q15 integers immediately
        current_phases[w] = (q15_t)((int32_t)(wave_shift * 10430.37f));
        phase_steps[w]    = (q15_t)((int32_t)(frequency * 10430.37f));

        // Pre-extract colors
        unsigned int base_color;
        if (w < (num_waves * BASS)) base_color = color1;
        else if (w < (num_waves * MID)) base_color = color2;
        else base_color = color3;

        r_bases[w] = EXTRACT_RED(base_color);
        g_bases[w] = EXTRACT_GREEN(base_color);
        b_bases[w] = EXTRACT_BLUE(base_color);
    }

    for (x = 0; x < OLED_DIM; x++) {

        // Yield for remote interrupt
        ButtonPress(ChangeMode); // TEMP CHANGE
        if (mode != WAVE) return;

        // ERASE PHASE
        for (w = 0; w < num_waves; w++) {
            int old_y = old_y_coords[w][x];
            if (old_y <= center) {
                drawFastVLine(x, old_y, center - old_y + 1, BLACK);
            } else {
                drawFastVLine(x, center, old_y - center + 1, BLACK);
            }
        }

        // Drawing phase
        for (w = 0; w < num_waves; w++) {

            // Advance the phase even if silent, so the wave doesn't desync!
            q15_t this_phase = current_phases[w];
            current_phases[w] += phase_steps[w]; // 1-cycle integer addition!

            if (wave_amps[w] < 2) continue; // Skip drawing silence

            // Pure integer CMSIS-DSP sine lookup
            q15_t sin_val = arm_sin_q15(this_phase);

            int offset = (int)((int32_t)wave_amps[w] * sin_val >> 15);
            int y = center + offset;

            if (y < 0) y = 0;
            if (y >= OLED_DIM) y = max_y;

            // Apply Y-axis gradient using pre-extracted bases
            unsigned char final_r = FADE_TO_BLACK(r_bases[w], y, max_y);
            unsigned char final_g = FADE_TO_BLACK(g_bases[w], y, max_y);
            unsigned char final_b = BLEND_TO_TARGET(b_bases[w], 255, y, max_y);
            unsigned int dynamic_color = Color565(final_r, final_g, final_b);

            if (y <= center) {
                drawFastVLine(x, y, center - y + 1, dynamic_color);
            } else {
                drawFastVLine(x, center, y - center + 1, dynamic_color);
            }

            old_y_coords[w][x] = y;
        }
    }
}

static void DrawPulse(size_t num_bins, q15_t* bin_peaks, uint16_t color1, uint16_t color2, uint16_t color3) {
    static uint8_t old_r_bass = 0, old_r_mid = 0, old_r_treble = 0;

        // Calculate Averages for the 3 Frequency Zones
        // BASS:
        int32_t bass_sum = 0;
        int bass_end = (num_bins * 2) / 10;
        if (bass_end < 1) bass_end = 1;
        int i;
        for(i = 0; i < bass_end; i++) { bass_sum += bin_peaks[i]; }
        int32_t bass_e = bass_sum / bass_end;

        // MIDS:
        int32_t mid_sum = 0;
        int mid_start = num_bins / 3;
        int mid_end = (num_bins * 2) / 3;
        for(i = mid_start; i < mid_end; i++) { mid_sum += bin_peaks[i]; }
        int32_t mid_e = mid_sum / (mid_end - mid_start);

        // TREBLE:
        // This captures the 'shimmer' without getting stuck in the dead air at the top
        int32_t treb_sum = 0;
        int treb_start = (num_bins * 7) / 10;
        for(i = treb_start; i < num_bins; i++) { treb_sum += bin_peaks[i]; }
        int32_t treb_e = treb_sum / (num_bins - treb_start);

        // Map to Radii using PULSE_BOOST
        uint8_t r_bass   = (bass_e * PULSE_BOOST) / MAX_MAGNITUDE;
        uint8_t r_mid    = (mid_e  * 55) / MAX_MAGNITUDE;
        uint8_t r_treble = (treb_e * 35) / MAX_MAGNITUDE; // Slightly larger for visibility

        if (r_bass > 63) r_bass = 63;
        if (r_mid > 63) r_mid = 63;
        if (r_treble > 63) r_treble = 63;

        // Erase and Draw (Dirty Overwrite)
        if (old_r_bass != r_bass)     drawCircle(OLED_DIM/2, OLED_DIM/2, old_r_bass, BLACK);
        if (old_r_mid != r_mid)       drawCircle(OLED_DIM/2, OLED_DIM/2, old_r_mid, BLACK);
        if (old_r_treble != r_treble) drawCircle(OLED_DIM/2, OLED_DIM/2, old_r_treble, BLACK);

        drawCircle(OLED_DIM/2, OLED_DIM/2, r_bass,   color1);
        drawCircle(OLED_DIM/2, OLED_DIM/2, r_mid,    color2);
        drawCircle(OLED_DIM/2, OLED_DIM/2, r_treble, color3);

        old_r_bass = r_bass; old_r_mid = r_mid; old_r_treble = r_treble;
}

void DrawVisuals(mode_t mode, size_t num_bins, q15_t* bin_peaks, uint16_t color1, uint16_t color2, uint16_t color3) {
    switch (mode) {
    case BAR:
        DrawBars(num_bins, bin_peaks, color1, color2, color3);
        break;
    case WAVE:
        DrawWaves(num_bins, bin_peaks, color1, color2, color3);
        break;
    case PULSE:
        DrawPulse(num_bins, bin_peaks, color1, color2, color3);
        break;
    default:
        Report("No mode selected. Cannot draw visuals\n\r");
        break;
    }

}
