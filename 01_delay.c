// delay.c
#include <LPC21xx.h>
#include "types.h"
#include "delay.h"

void delay_us(u32 dlyUS)
{
    dlyUS *= 12;    // Approximate microsecond delay for 12 MHz
    while(dlyUS--);
}

void delay_ms(u32 dlyMS)
{
    dlyMS *= 12000; // Approximate millisecond delay
    while(dlyMS--);
}

void delay_s(u32 dlyS)
{
    dlyS *= 12000000; // Approximate 1-second delay
    while(dlyS--);
}
