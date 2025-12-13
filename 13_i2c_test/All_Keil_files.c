// types.h
#ifndef TYPES_H
#define TYPES_H

typedef unsigned char  u8;
typedef signed char    s8;
typedef unsigned short u16;
typedef signed short   s16;
typedef unsigned long  u32;
typedef signed long    s32;
typedef float          f32;
typedef double         f64;

#endif

****************************************
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
**********************************************
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
************************************************************
#ifndef _UART1_H_
#define _UART1_H_

void UART1_Init(void);
void UART1_TxChar(unsigned char ch);
void UART1_TxString(unsigned char *str);

extern volatile unsigned char rfid_buff[20];
extern volatile unsigned char r_flag;
extern volatile unsigned char r_index;

#endif
******************************************************
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
void UART1_isr(void) __irq;
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
***************************************************************
//lcd_defines.h
//HD44780 Commands 
#define CLEAR_LCD            0x01 
#define RET_CUR_HOME         0x02 
#define SHIFT_CUR_RIGHT      0x06 
#define SHIFT_CUR_LEFT       0x07 
#define DSP_OFF              0x08 
#define DSP_ON_CUR_OFF       0x0c 
#define DSP_ON_CUR_ON        0x0E 
#define DSP_ON_CUR_BLINK     0x0F 
#define SHIFT_DISP_LEFT      0x10 
#define SHIFT_DISP_RIGHT     0x14 
#define MODE_8BIT_1LINE      0x30 
#define MODE_4BIT_1LINE      0x20 
#define MODE_8BIT_2LINE      0x38 
#define MODE_4BIT_2LINE      0x28 
#define GOTO_LINE1_POS0      0x80 
#define GOTO_LINE2_POS0      0xC0 
#define GOTO_LINE3_POS0      0x94 
#define GOTO_LINE4_POS0      0xD4 
#define GOTO_CGRAM_START     0x40 

//lpc2129 port pin connection to lcd
#define LCD_DATA        10//@p0.8(d0) to p0.15(d7)
#define LCD_RS         18//@p0.16
#define LCD_RW         19//@p0.17
#define LCD_EN         20//@p0.18
************************************************************
//lcd.h
#include "types.h"
void Init_LCD(void);
void WriteLCD(u8 byte);
void CmdLCD(u8 cmd);
void CharLCD(u8 asciiVal);
void StrLCD(s8 *);
void U32LCD(u32);
void S32LCD(s32);
void F32LCD(f32,u32);
void BuildCGRAM(u8 *,u8 );
***************************************
//lcd.c
#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "lcd_defines.h"
#include "lcd.h"
#include "delay.h"

void WriteLCD(u8 byte)
{
	//select write operation
	IOCLR0=1<<LCD_RW;
	//write byte to lcd data pins
	WRITEBYTE(IOPIN0,LCD_DATA,byte);
	//provide high to low pulse
	IOSET0=1<<LCD_EN;
	delay_us(1);
	IOCLR0=1<<LCD_EN;
	delay_ms(2);
}

void CmdLCD(u8 cmdByte)
{
	//set rs pin for cmd/inst reg
  IOCLR0=1<<LCD_RS;
  //write cmd byte to cmd reg
  WriteLCD(cmdByte); 	
}

void Init_LCD(void)
{
	//cfg p0.8(d0) to p0.15(d7),
	//p0.16(rs),p0.17(rw),p0.18(en) 
	//as gpio output pins
	IODIR0|=((0xFF<<LCD_DATA)|
	         (1<<LCD_RS)|(1<<LCD_RW)|(1<<LCD_EN));
	
	//power on delay
	delay_ms(15);
	CmdLCD(0x30);
	delay_ms(4);
	delay_us(100);
	CmdLCD(0x30);
	delay_us(100);
	CmdLCD(0x30);
	CmdLCD(MODE_8BIT_2LINE);
	CmdLCD(DSP_ON_CUR_ON);
	CmdLCD(CLEAR_LCD);
	CmdLCD(SHIFT_CUR_RIGHT);
}

void CharLCD(u8 asciiVal)
{
	//sel data reg
	IOSET0=1<<LCD_RS;
	//write ascii value via data reg to ddram
	WriteLCD(asciiVal);
}

void StrLCD(s8 *str)
{
	while(*str)
		CharLCD(*str++);
}

void U32LCD(u32 n)
{
	s32 i=0;
	u8 a[10];
	
	if(n==0)
	{
		CharLCD('0');
	}
	else
	{
	  while(n>0)
    {
		  a[i++]=(n%10)+48;
      n/=10;			
    }
    for(--i;i>=0;i--)
      CharLCD(a[i]);		
	}
}

void S32LCD(s32 num)
{
	if(num<0)
	{
		CharLCD('-');
		num=-num;
	}
	U32LCD(num);
}

void F32LCD(f32 fnum,u32 nDP)
{
	u32 num,i;
	if(fnum<0.0)
	{
		CharLCD('-');
		fnum=-fnum;
	}
	num=fnum;
	U32LCD(num);
	CharLCD('.');
	for(i=0;i<nDP;i++)
	{
		fnum=(fnum-num)*10;
		num=fnum;
		CharLCD(num+48);
	}
}

void BuildCGRAM(u8 *p,u8 nBytes)
{
	u32 i;
	//point to cgram start
	CmdLCD(GOTO_CGRAM_START);
	//select data reg
	IOSET0=1<<LCD_RS;
	
	for(i=0;i<nBytes;i++)
	{
		//write to cgram vi data reg
		WriteLCD(p[i]);
	}
	//point back to ddram start/display
	CmdLCD(GOTO_LINE1_POS0);
}
*****************************************
// delay.h
#ifndef DELAY_H
#define DELAY_H

#include "types.h"

void delay_us(u32 dlyUS);
void delay_ms(u32 dlyMS);
void delay_s(u32 dlyS);

#endif

*********************************
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
*************************************
#ifndef DEFINES_H
#define DEFINES_H

#define WRITEBYTE(REG,BIT,VAL)   REG = (REG & ~(0xFF<<BIT)) | ((VAL&0xFF)<<BIT)

#endif
*******************************
#include <LPC214x.h>
#include "uart1.h"
#include "i2c.h"
#include "lcd_defines.h"
#include "lcd.h"
#include "delay.h"

#define AT24C256_ADDR  0x50
#define EEPROM_START   0x0000

unsigned char eeprom_buf[20];

int main(void)
{
    Init_LCD();
    UART1_Init();
    i2c_init();

    StrLCD("Scan RFID");

    while(1)
    {
        if(r_flag == 2)
        {
            r_flag = 0;

             CmdLCD(0x01);
            StrLCD("RFID:");
					  CmdLCD(0xC0);
            StrLCD((s8*)rfid_buff);

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
            CmdLCD(0x01);
            StrLCD("EEPROM:");
            CmdLCD(0x01);
            StrLCD((s8*)eeprom_buf);

            r_index = 0;
        }
    }
}
