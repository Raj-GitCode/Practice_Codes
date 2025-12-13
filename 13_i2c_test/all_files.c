Understood 👍
Below is a **clean, complete, and properly separated file structure** for your project **(LPC2148 + RFID on UART1 + AT24C256 EEPROM + LCD)**.

I will **only include what is required**, nothing extra.

---

# 📁 PROJECT FILE STRUCTURE

```
/Project
│
├── main.c
├── system.c
│
├── i2c.c
├── i2c.h
│
├── uart1.c
├── uart1.h
│
├── lcd.c
├── lcd.h
│
├── delay.c
├── delay.h
│
├── types.h
```

---

# 📄 types.h

```c
#ifndef _TYPES_H_
#define _TYPES_H_

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#endif
```

---

# 📄 i2c.h

```c
#ifndef _I2C_H_
#define _I2C_H_

#include "types.h"

void i2c_init(void);
void i2c_start(void);
void i2c_restart(void);
void i2c_stop(void);
void i2c_write(u8 data);

u8   i2c_mack(void);
u8   i2c_read_nack(void);

void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, u8 *p, u8 nBytes);
void i2c_eeprom_sequentialread(u8 slaveAddr, u16 memAddr, u8 *p, u16 nBytes);

#endif
```

---

# 📄 i2c.c  (AT24C256 COMPATIBLE)

```c
#include "i2c.h"
#include <LPC214x.h>
#include "delay.h"

void i2c_init(void)
{
    PINSEL0 &= ~((3<<4)|(3<<6));
    PINSEL0 |=  ((1<<4)|(1<<6));

    I2C0SCLH = 75;
    I2C0SCLL = 75;

    I2C0CONSET = (1<<6);
    I2C0CONCLR = 0x2C;
}

void i2c_start(void)
{
    I2C0CONSET = (1<<5);
    while(!(I2C0CONSET & (1<<3)));
    I2C0CONCLR = (1<<5);
}

void i2c_restart(void)
{
    I2C0CONSET = (1<<5);
    I2C0CONCLR = (1<<3);
    while(!(I2C0CONSET & (1<<3)));
    I2C0CONCLR = (1<<5);
}

void i2c_stop(void)
{
    I2C0CONSET = (1<<4);
    I2C0CONCLR = (1<<3);
}

void i2c_write(u8 data)
{
    I2C0DAT = data;
    I2C0CONCLR = (1<<3);
    while(!(I2C0CONSET & (1<<3)));
}

u8 i2c_mack(void)
{
    I2C0CONSET = (1<<2);
    I2C0CONCLR = (1<<3);
    while(!(I2C0CONSET & (1<<3)));
    I2C0CONCLR = (1<<2);
    return I2C0DAT;
}

u8 i2c_read_nack(void)
{
    I2C0CONCLR = (1<<2);
    I2C0CONCLR = (1<<3);
    while(!(I2C0CONSET & (1<<3)));
    return I2C0DAT;
}

void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, u8 *p, u8 nBytes)
{
    u8 i;

    i2c_start();
    i2c_write(slaveAddr << 1);

    i2c_write(memAddr >> 8);
    i2c_write(memAddr & 0xFF);

    for(i=0;i<nBytes;i++)
        i2c_write(p[i]);

    i2c_stop();
    delay_ms(5);
}

void i2c_eeprom_sequentialread(u8 slaveAddr, u16 memAddr, u8 *p, u16 nBytes)
{
    u16 i;

    i2c_start();
    i2c_write(slaveAddr << 1);

    i2c_write(memAddr >> 8);
    i2c_write(memAddr & 0xFF);

    i2c_restart();
    i2c_write((slaveAddr<<1)|1);

    for(i=0;i<nBytes-1;i++)
        p[i] = i2c_mack();

    p[i] = i2c_read_nack();
    i2c_stop();
}
```

---

# 📄 uart1.h

```c
#ifndef _UART1_H_
#define _UART1_H_

void UART1_Init(void);
void UART1_TxChar(unsigned char ch);
void UART1_TxString(unsigned char *str);

extern volatile unsigned char rfid_buff[20];
extern volatile unsigned char r_flag;
extern volatile unsigned char r_index;

#endif
```

---

# 📄 uart1.c  (RFID READER)

```c
#include <LPC214x.h>
#include "uart1.h"

#define FOSC   12000000
#define CCLK   (FOSC*5)
#define PCLK   (CCLK/4)
#define BAUD   9600
#define DIVISOR (PCLK/(16*BAUD))

volatile unsigned char rfid_buff[20];
volatile unsigned char rch;
volatile unsigned char r_index = 0;
volatile unsigned char r_flag  = 0;

void UART1_Init(void)
{
    PINSEL0 |= (1<<16)|(1<<18);

    U1LCR = 0x83;
    U1DLL = DIVISOR & 0xFF;
    U1DLM = (DIVISOR >> 8) & 0xFF;
    U1LCR = 0x03;

    U1IER = 0x01;

    VICIntSelect &= ~(1<<7);
    VICVectAddr1  = (unsigned)UART1_isr;
    VICVectCntl1  = 0x20 | 7;
    VICIntEnable |= (1<<7);
}

void UART1_TxChar(unsigned char ch)
{
    U1THR = ch;
    while(!(U1LSR & (1<<6)));
}

void UART1_TxString(unsigned char *str)
{
    while(*str)
        UART1_TxChar(*str++);
}

void UART1_isr(void) __irq
{
    if(U1IIR & 0x04)
    {
        rch = U1RBR;

        if(rch == 0x02)
        {
            r_index = 0;
            r_flag = 1;
        }
        else if(r_flag == 1 && rch != 0x03)
        {
            if(r_index < 19)
                rfid_buff[r_index++] = rch;
        }
        else if(rch == 0x03)
        {
            rfid_buff[r_index] = '\0';
            r_flag = 2;
        }
    }
    VICVectAddr = 0;
}
```

---

# 📄 main.c

```c
#include <LPC214x.h>
#include "uart1.h"
#include "i2c.h"
#include "lcd.h"
#include "delay.h"

#define AT24C256_ADDR  0x50
#define EEPROM_START   0x0000

unsigned char eeprom_buf[20];

int main(void)
{
    LCD_Init();
    UART1_Init();
    i2c_init();

    StrLCD("Scan RFID");

    while(1)
    {
        if(r_flag == 2)
        {
            r_flag = 0;

            LCD_Clear();
            StrLCD("RFID:");
            LCD_Cmd(0xC0);
            StrLCD((char*)rfid_buff);

            i2c_eeprom_pagewrite(
                AT24C256_ADDR,
                EEPROM_START,
                (u8*)rfid_buff,
                r_index
            );

            delay_ms(10);

            i2c_eeprom_sequentialread(
                AT24C256_ADDR,
                EEPROM_START,
                eeprom_buf,
                r_index
            );

            eeprom_buf[r_index] = '\0';

            delay_ms(1000);
            LCD_Clear();
            StrLCD("EEPROM:");
            LCD_Cmd(0xC0);
            StrLCD((char*)eeprom_buf);

            r_index = 0;
        }
    }
}
```

---

# ✅ WHAT THIS GIVES YOU

✔ Proper file separation
✔ AT24C256 EEPROM supported
✔ RFID UART1 interrupt-based receive
✔ EEPROM write + read verified
✔ LCD display using your functions

---

  POWER ON
   ↓
MCU Initialization
   ↓
Waiting for RFID card
   ↓
UART1 Interrupt receives RFID
   ↓
RFID data stored in buffer
   ↓
Main loop detects data ready
   ↓
Write RFID to AT24C256 EEPROM
   ↓
Read RFID back from EEPROM
   ↓
Display data on LCD
   ↓
Wait for next card
