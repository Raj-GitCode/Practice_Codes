#include <LPC21xx.h>

volatile unsigned char flag = 0;  // Interrupt flag

void myISR(void) __irq;

int main()
{
    IODIR0 |= (1 << 25);          // P0.25 as output (LED)

    // EINT1 on P0.3
    PINSEL0 |= (3 << 6);          // Select EINT1
    EXTMODE |= (1 << 1);          // Edge-sensitive
    EXTPOLAR &= ~(1 << 1);        // Falling edge
    EXTINT = (1 << 1);            // Clear any pending interrupt

    VICIntEnable = (1 << 15);     // Enable EINT1 in VIC
    VICVectCntl0 = (1 << 5) | 15; // Slot0, enable, EINT1
    VICVectAddr0 = (unsigned long)myISR;

    while(1)
    {
        if(flag == 1)  // Check if interrupt occurred
        {
            IOPIN0 ^= (1 << 25);  // Toggle LED
            flag = 0;             // Clear flag
        }
    }
}

// ISR just sets the flag
void myISR(void) __irq
{
    flag = 1;           // Set interrupt flag
    EXTINT = (1 << 1);  // Clear external interrupt
    VICVectAddr = 0;    // Acknowledge interrupt
}
