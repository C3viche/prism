#ifndef AWS_PARSER_H
#define AWS_PARSER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Configuration
#define MAX_ESP_LENGTH 512

// The Data Structure
typedef struct {
    char bars[16];
    char c1[16];
    char c2[16];
    char c3[16];
    char grav[16];
    char rate[16];
} CC3200_Data;

// Library Functions
int FetchInput(char *finalMsg);
int ProcessIncomingData(char *msg);

#endif // AWS_PARSER_H
