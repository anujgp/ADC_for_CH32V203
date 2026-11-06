#include <stdio.h>
#include "clock.h"
#include "ADC.h"
#include "ch32fun.h"

static volatile uint16_t channelValue=0;

int main(void)
{
    Clock_Init();
    ADC_Init();
    ADC_AddChannel(0); // Add channel 0 (PA0)
    ADC_SetSample(0, ADC_SAMPLE_55_5); // Set sample time for channel 0
    while (1)
    {
        ADC_start();
        channelValue =  ADC_GetValue(0); // Read value from channel 0
    }
    return 0;
}