#include "ch32fun.h"
#include "clock.h"

void Clock_Init(void)
{
    RCC->CTLR |= RCC_HSEON;            // Enable HSE
    while(!(RCC->CTLR & RCC_HSERDY));
    RCC->CFGR0 &= ~RCC_PLLXTPRE;     // HSE not divided
    RCC->CFGR0 |= RCC_PLLSRC;        // PLL source = HSE
    RCC->CFGR0 &= ~(0xF << 18);
    RCC->CFGR0 |= (0x7 << 18);       // PLL multiplier = ×9 (8 MHz → 72 MHz)
    RCC->CFGR0 &= ~RCC_HPRE;         // AHB = SYSCLK
    RCC->CFGR0 &= ~RCC_PPRE1;        // APB1 = HCLK
    RCC->CFGR0 &= ~RCC_PPRE2;        // APB2 = HCLK
    RCC->CTLR |= RCC_PLLON;          // Enable PLL
    while(!(RCC->CTLR & RCC_PLLRDY)); // wait for pll
    RCC->CFGR0 &= ~RCC_SW;            // Switch system clock to PLL
    RCC->CFGR0 |= RCC_SW_PLL;
    while((RCC->CFGR0 & RCC_SWS) != RCC_SWS_PLL);   //wait for switch
    RCC->CFGR0 |= (0x02 << 14);// adcpre= b10(=/6)
    RCC->APB2PCENR |= (1 << 9);// Bit 9: Enable operational clock gate for ADC1
    RCC->APB2PCENR |= (1 << 2);// Bit 2: Enable operational clock for Port A
    RCC->AHBPCENR  |= (1 << 0);// Enable DMA1 clock
}