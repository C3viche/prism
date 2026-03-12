#ifndef AWS_PARSER_H
#define AWS_PARSER_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Configuration
#define MAX_ESP_LENGTH 512

// The Data Structure
typedef struct {
    char bars[16];
    uint16_t c1;
    uint16_t c2;
    uint16_t c3;
    char grav[16];
    uint16_t rate;
} CC3200_Data;

// Library Functions
int FetchInput(char *finalMsg);
int ProcessIncomingData(char *msg, CC3200_Data *data);

#endif // AWS_PARSER_H
