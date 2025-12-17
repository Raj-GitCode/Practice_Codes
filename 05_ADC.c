//adc_defines.h
#ifndef ADC_DEFINES_H
#define ADC_DEFINES_H

// System clocks
#define FOSC 12000000
#define CCLK (FOSC * 5)       // 60 MHz
#define PCLK (CCLK / 4)       // 15 MHz
#define ADCCLK 3000000
#define CLKDIV ((PCLK / ADCCLK) - 1)

// ADC register bit defines
#define CLKDIV_BITS    8
#define PDN_BIT        21
#define START_BIT      24
#define RESULT_BITS     6
#define DONE_BIT       31

// ADC channel mapping
#define ADC0_PINSEL   (1 << 24)   // P0.28 -> AD0.1

#endif

***********************************************
// ADC Initialization
void Init_ADC(void);

// Read ADC channel (0–7), returns 10-bit value
unsigned int Read_ADC(unsigned int ch);
**************************************************
//adc.c
#include <LPC214x.h>
#include "adc.h"
#include "adc_defines.h"

void Init_ADC(void)
{
		 // Configure P0.28 as AD0.1 input
    PINSEL1 &= ~(3 << 24);  // Clear bits 24-25
    PINSEL1 |= ADC0_PINSEL;

    // Power up ADC and set clock divider
    AD0CR = (1 << PDN_BIT) | (CLKDIV << CLKDIV_BITS);
}

unsigned int Read_ADC(unsigned int ch)
{
    unsigned int result;

    // Clear previous channel selection (bits 0–7)
    AD0CR &= 0xFFFFFF00;
    AD0CR |= (1 << ch);             // Select channel
    AD0CR |= (1 << START_BIT);      // Start conversion

    // Wait until conversion is done
    while (((AD0GDR >> DONE_BIT) & 1) == 0);

    // Extract 10-bit result
    result = (AD0GDR >> RESULT_BITS) & 0x3FF;

    // Stop conversion (clear START bits 24-26)
    AD0CR &= ~(0x07 << 24);

    return result;
}

*************************************888
  void Show_ADC_Temperature(void)
{
    unsigned int adcValue;
    float voltage, temperature;
    int intPart, decPart;

    adcValue = Read_ADC(1);
    voltage = (adcValue * 3.3f) / 1023.0f;
    temperature = voltage * 100.0f;

    intPart = (int)temperature;
    decPart = (int)((temperature - intPart) * 10 + 0.5f);

    CmdLCD(GOTO_LINE1_POS0 + 11);
    U32LCD(intPart);
    CharLCD('.');
    U32LCD(decPart);
    CharLCD(223);
    StrLCD("C");
}
