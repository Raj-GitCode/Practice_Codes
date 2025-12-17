#include <LPC21xx.h>

typedef unsigned int u32;
typedef unsigned char u8;

u8 seg_code[10] =
{
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

void delay_ms(u32 ms)
{
    u32 i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 6000; j++);
}

int main(void)
{
    u32 i;
    IODIR0 |= 0xFF;

    while(1)
    {
        for(i = 0; i <= 9; i++)
        {
            IOCLR0 = 0xFF;           // Clear previous value
            IOSET0 = seg_code[i];   // Display digit
            delay_ms(500);          // 0.5 second delay
        }
    }
}

**********************************************************
  With Shifitng....
  
int main(void)
{
  u32 i;

  IODIR0 |= (0xFF << SEG_PINS);

  while(1)
  {
    for(i = 0; i < 10; i++)
    {
      IOCLR0 = (0xFF << SEG_PINS);           
      IOSET0 = (segLUT[i] << SEG_PINS);     
      delay_ms(1000);                       
    }
  }
}
***********************************************
7Seg with keypad............
//this code is for taking input from keypad and show on segment
int main(void)
{
    char key;
    u32 digit;

    /* Configure P0.8 – P0.15 as output */
    IODIR0 |= (0xFF << SEG_PINS);

    while(1)
    {
        key = KeyScan();      // Get key from keypad

        if(key >= '0' && key <= '9')   // valid digit?
        {
            digit = key - '0';         // ASCII ? number

            IOCLR0 = (0xFF << SEG_PINS);            // Clear segments
            IOSET0 = (segLUT[digit] << SEG_PINS);  // Display digit
        }
    }
}

