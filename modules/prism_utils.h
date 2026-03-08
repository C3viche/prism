/*
 * prism_utils.h
 *
 *  Created on: Mar 7, 2026
 *      Author: crisemble
 */

#ifndef MODULES_PRISM_UTILS_H_
#define MODULES_PRISM_UTILS_H_

#include <stdio.h>

// The CC3200’s Analog-to-Digital Converter (ADC) only understands positive voltages.
// It has a 12-bit resolution, meaning it translates voltage into a strict scale from 0 to 4095
#define MAX_ADC_VALUE ((1 << 12) - 1)
#define ADC_DC_OFFSET (1 << 11)



#endif /* MODULES_PRISM_UTILS_H_ */
