#ifndef ADC_MIC_H
#define ADC_MIC_H

#include <stdint.h>
#include "arm_math.h"

// Buffer Status
#define BUFFER_NONE  0
#define BUFFER_PING  1
#define BUFFER_PONG  2

void SetupADCMic(uint32_t sampleRate);
void StartADCSampling(q15_t *ping_buffer, q15_t *pong_buffer, uint32_t window_size);
int  CheckBufferReady(); // Returns BUFFER_PING, BUFFER_PONG, or BUFFER_NONE
void ClearBufferFlag(int buffer_id);

#endif
