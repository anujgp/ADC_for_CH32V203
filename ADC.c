#include "ADC.h"
#include "ch32fun.h"


#define MAX_ADC_CHANNELS 16

static uint8_t  channelList[MAX_ADC_CHANNELS];
static volatile uint16_t convertedValues[MAX_ADC_CHANNELS];
static volatile uint8_t  channelCount = 0;
static volatile uint8_t  currentIndex = 0;
static volatile bool  adcRunning = false;

/* Helper: Configure corresponding GPIO pin to Analog Input */
static void ADC_PinConfig(uint8_t channel)
{
    if (channel <= 7) // PA0 - PA7 (Channels 0 - 7)
    {
        RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
        GPIOA->CFGLR &= ~(0x0F << (channel * 4)); // MODE=00, CNF=00 (Analog)
    }
    else if (channel == 8 || channel == 9) // PB0, PB1 (Channels 8 - 9)
    {
        RCC->APB2PCENR |= RCC_APB2Periph_GPIOB;
        GPIOB->CFGLR &= ~(0x0F << ((channel - 8) * 4));
    }
    else if (channel >= 10 && channel <= 15) // PC0 - PC5 (Channels 10 - 15)
    {
        RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
        GPIOC->CFGLR &= ~(0x0F << ((channel - 10) * 4));
    }
}

void ADC_Init(void)
{
    /* 1. Turn on clocks for ADC1 */
    RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

    /* 2. Set ADCCLK divider: PCLK2 / 6 (72 MHz / 6 = 12 MHz <= 14 MHz max) */
    RCC->CFGR0 &= ~RCC_ADCPRE;
    RCC->CFGR0 |= RCC_ADCPRE_DIV6;

    /* 3. Wake up ADC peripheral */
    ADC1->CTLR2 |= ADC_ADON;
    Delay_Us(10); // Calibration requires stabilization delay

    /* 4. Reset & run hardware calibration */
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL);

    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL);

    /* 5. Enable End-of-Conversion Interrupt */
    ADC1->CTLR1 |= ADC_EOCIE;

    /* 6. Enable ADC interrupt in PFIC (IRQ34 is ADC1_2_IRQn) */
    NVIC_EnableIRQ(ADC1_2_IRQn);
}

bool ADC_AddChannel(uint8_t channel)
{
    if (channelCount >= MAX_ADC_CHANNELS || channel > 17)
    {
        return false;
    }

    ADC_PinConfig(channel);
    ADC_SetSample(channel, ADC_SAMPLE_55_5); // Sensible default sample time

    channelList[channelCount] = channel;
    channelCount++;
    return true;
}

void ADC_SetSample(uint8_t channel, uint8_t sampleTime)
{
    sampleTime &= 0x07;
    if (channel <= 9)
    {
        ADC1->SAMPTR2 &= ~(0x07 << (channel * 3));
        ADC1->SAMPTR2 |=  (sampleTime << (channel * 3));
    }
    else if (channel <= 17)
    {
        ADC1->SAMPTR1 &= ~(0x07 << ((channel - 10) * 3));
        ADC1->SAMPTR1 |=  (sampleTime << ((channel - 10) * 3));
    }
}

void ADC_Start(void)
{
    if (channelCount == 0) return;

    adcRunning = true;
    currentIndex = 0;

    /* Single regular conversion sequence length (L = 0 means 1 conversion) */
    ADC1->RSQR1 &= ~ADC_L;
    ADC1->RSQR3 = (channelList[0] & 0x1F);

    /* Software trigger selection: EXTSEL = 111b */
    ADC1->CTLR2 |= ADC_EXTSEL;
    ADC1->CTLR2 |= ADC_EXTTRIG;

    /* Clear any lingering status flag, then trigger conversion */
    ADC1->STATR = 0;
    ADC1->CTLR2 |= ADC_SWSTART;
}

void ADC_Stop(void)
{
    adcRunning = false;
    ADC1->CTLR2 &= ~ADC_SWSTART;
}

void ADC_ClearChannels(void)
{
    ADC_Stop();
    channelCount = 0;
    currentIndex = 0;
}

uint16_t ADC_GetValue(uint8_t channel)
{
    if (channel < MAX_ADC_CHANNELS)
    {
        return convertedValues[channel];
    }
    return 0;
}

/* ISR attribute compatible with ch32fun fast interrupt stack */
void ADC1_2_IRQHandler(void) __attribute__((interrupt));
void ADC1_2_IRQHandler(void)
{
    if (ADC1->STATR & ADC_EOC)
    {
        /* Reading RDATAR automatically clears the EOC flag */
        uint16_t val = (uint16_t)(ADC1->RDATAR & 0x0FFF);
        convertedValues[channelList[currentIndex]] = val;

        if (!adcRunning)
        {
            return;
        }

        /* Advance to next channel in list */
        currentIndex++;
        if (currentIndex >= channelCount)
        {
            currentIndex = 0;
        }

        /* Queue the next channel in Rank 1 */
        ADC1->RSQR3 = (channelList[currentIndex] & 0x1F);

        /* Trigger next conversion */
        ADC1->CTLR2 |= ADC_SWSTART;
    }
}