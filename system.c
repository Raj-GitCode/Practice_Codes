
/*
//sysem.c
#include <LPC214x.h>
#include "lcd.h"
#include "lcd_defines.h"
#include "types.h"
#include "delay.h"
#include "i2c.h"
#include "uart1.h"
#include "kpm.h"
#include <string.h>

#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)
#define BAUD 9600
#define DIVISOR (PCLK/(16*BAUD))

#define EEPROM_ADDR 0x50

//-----------------------------------------------------
// UART0 Functions (for GSM Module)
//-----------------------------------------------------
void UART0_Init(void)
{
    PINSEL0 &= ~(0xF);
    PINSEL0 |= 0x05;  // TXD0, RXD0 enabled

    U0LCR = 0x83;
    U0DLL = DIVISOR & 0xFF;
    U0DLM = (DIVISOR >> 8) & 0xFF;
    U0LCR = 0x03;
}

void UART0_TxChar(unsigned char ch)
{
    while(!(U0LSR & (1<<5)));
    U0THR = ch;
}

void UART0_TxString(char *p)
{
    while(*p)
        UART0_TxChar(*p++);
}

//-----------------------------------------------------
// OTP GENERATION
//-----------------------------------------------------
u16 seed = 1234;

void Generate_OTP(u8 *otp)
{
    seed = seed * 7 + 3;       // simple random-like formula
    seed = seed % 10000;       // keep in 4-digit range

    otp[0] = '0' + (seed / 1000);
    otp[1] = '0' + ((seed / 100) % 10);
    otp[2] = '0' + ((seed / 10) % 10);
    otp[3] = '0' + (seed % 10);
    otp[4] = '\0';
}

//-----------------------------------------------------
// SEND OTP USING GSM
//-----------------------------------------------------
void Send_OTP_SMS(u8 *otp)
{
    UART0_TxString("AT\r\n");
    delay_ms(50);

    UART0_TxString("ATE0\r\n");
    delay_ms(50);

    UART0_TxString("AT+CMGF=1\r\n");
    delay_ms(50);

    UART0_TxString("AT+CMGS=\"1234567890\"\r\n");
    delay_ms(50);

    UART0_TxString("Your OTP is: ");
    UART0_TxString((char*)otp);
    delay_ms(50);

    UART0_TxChar(0x1A);
    delay_ms(200);
}

//-----------------------------------------------------
// GET OTP FROM KEYPAD
//-----------------------------------------------------
void Get_OTP_FromKeypad(u8 *buf)
{
    u8 key;
    u8 i = 0;

    CmdLCD(CLEAR_LCD);
    StrLCD("Enter OTP");
    CmdLCD(0xC0);

    while(i < 4)
    {
        key = KeyScan();
        if(key != '\0')
        {
            buf[i++] = key;
            CharLCD('*');
            delay_ms(250);
        }
    }
    buf[4] = '\0';
}

//-----------------------------------------------------
// SYSTEM INIT
//-----------------------------------------------------
void System_Init(void)
{
    UART1_Init();
    UART0_Init();
    Init_LCD();
    Init_KPM();
    i2c_init();

		PINSEL1 &= ~(3 << 18);      // Clear bits for P0.25 (PINSEL1 bits 18–19)

    // Set P0.25 as OUTPUT
    IODIR0 |= (1 << 25);
	
	
    // Write default RFID tag (8 bytes)
    i2c_eeprom_pagewrite(EEPROM_ADDR, 0x00, (u8*)"12345678", 8);
    delay_ms(10);

    // Default password
    i2c_eeprom_pagewrite(EEPROM_ADDR, 0x20, (u8*)"12341234", 8);
    delay_ms(10);
}

//-----------------------------------------------------
// RFID READ
//-----------------------------------------------------
void RFID_ReadTag(u8 *tag)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("Show RFID Tag");

    UART1_TxString("Waiting for RFID...\r\n");
    UART1_RxString(tag, 20);

    UART1_TxString("RFID: ");
    UART1_TxString(tag);
    UART1_TxString("\r\n");
}

//-----------------------------------------------------
// CHECK RFID TAG MATCH
//-----------------------------------------------------
u8 Check(u8 *tag)
{
    u8 stored[9];
    i2c_eeprom_sequentialread(EEPROM_ADDR, 0x00, stored, 8);
    stored[8] = '\0';

    return (strcmp((char*)tag, (char*)stored) == 0);
}

//-----------------------------------------------------
// VERIFY PASSWORD
//-----------------------------------------------------
u8 VerifyPassword()
{
    u8 pass[9], stored[9], key, i=0;

    CmdLCD(CLEAR_LCD);
    StrLCD("Enter Password");
    CmdLCD(0xC0);

    while(i < 8)
    {
        key = KeyScan();
        if(key != '\0')
        {
            pass[i++] = key;
            CharLCD('*');
            delay_ms(250);
        }
    }
    pass[8] = '\0';

    i2c_eeprom_sequentialread(EEPROM_ADDR, 0x20, stored, 8);
    stored[8] = '\0';

    if(strcmp((char*)pass, (char*)stored) == 0)
    {
        CmdLCD(CLEAR_LCD);
        StrLCD("Password OK");
        delay_s(1);
        return 1;
    }

    CmdLCD(CLEAR_LCD);
    StrLCD("Wrong Password");
    delay_s(2);

    return 0;
}

u8 Get_OTP_WithTimeout(u8 *buf, u16 timeout_seconds)
{
    u8 key;
    u8 i = 0;
    u16 elapsed = 0;

    CmdLCD(CLEAR_LCD);
    StrLCD("Enter OTP");
    CmdLCD(0xC0);

    while(i < 4)
    {
        key = KeyScan();
        if(key != '\0')
        {
            buf[i++] = key;
            CharLCD('*');
            delay_ms(200);
        }

        delay_ms(100);       // check every 100 ms
        elapsed++;

        if(elapsed >= (timeout_seconds * 10)) // 10 intervals per second
        {
            return 0;   // TIMEOUT
        }
    }

    buf[4] = '\0';
    return 1;   // success
}
u8 VerifyOTP(void)
{
    u8 otp[5];
    u8 entered[5];

    Generate_OTP(otp);

    CmdLCD(CLEAR_LCD);
    StrLCD("Sending OTP...");
    Send_OTP_SMS(otp);
    delay_s(1);

    // 20-second timeout
    if(!Get_OTP_WithTimeout(entered, 5))
    {
        CmdLCD(CLEAR_LCD);
        StrLCD("TIMEOUT....!");
				CmdLCD(GOTO_LINE2_POS0+3);
				StrLCD("ACESS DENIED");
        delay_s(2);
        return 0;
    }

    if(strcmp((char*)otp, (char*)entered) == 0)
    {
        CmdLCD(CLEAR_LCD);
        StrLCD("OTP Verified");
        delay_s(1);
        return 1;
    }

    CmdLCD(CLEAR_LCD);
    StrLCD("Wrong OTP");
    delay_s(2);

    return 0;
}

void GrantAccess(void)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("ACCESS GRANTED");
    CmdLCD(0xC0);
    StrLCD("Welcome EMP1");
    delay_s(2);
}
void ShowInvalidCard(u8 *tag)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("INVALID CARD");
    CmdLCD(0xC0);
    StrLCD((s8*)tag);
    delay_s(2);
}
void LED_ON(void)
{
    IOSET0 = (1 << 25);   
	  delay_s(2);
	  IOCLR0=(1<<25);
	  
}
*/


// system.c
#include <LPC214x.h>
#include "lcd.h"
#include "lcd_defines.h"
#include "types.h"
#include "delay.h"
#include "i2c.h"
#include "uart1.h"
#include "kpm.h"
#include <string.h>

#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)
#define BAUD 9600
#define DIVISOR (PCLK/(16*BAUD))

#define EEPROM_ADDR 0x50

/* ----------------- RFID shared globals (used by ISR and main) ----------------- */
/* Define them here (storage). uart1.h declares them extern so uart1.c can use them. */
volatile unsigned char rfid_buff[20];
volatile unsigned char r_index = 0;
volatile unsigned char r_flag = 0;   /* 0 = no data, 1 = receiving, 2 = ready */
volatile unsigned char rch = 0;

/*-----------------------------------------------------
   UART0 Functions (for GSM Module)
  -----------------------------------------------------*/
void UART0_Init(void)
{
    PINSEL0 &= ~(0xF);
    PINSEL0 |= 0x05;  /* TXD0, RXD0 enabled */

    U0LCR = 0x83;
    U0DLL = DIVISOR & 0xFF;
    U0DLM = (DIVISOR >> 8) & 0xFF;
    U0LCR = 0x03;
}

void UART0_TxChar(unsigned char ch)
{
    while(!(U0LSR & (1<<5)));
    U0THR = ch;
}

void UART0_TxString(char *p)
{
    while(*p)
        UART0_TxChar(*p++);
}

/*-----------------------------------------------------
   OTP GENERATION
  -----------------------------------------------------*/
u16 seed = 1234;

void Generate_OTP(u8 *otp)
{
    seed = seed * 7 + 3;       // simple random-like formula
    seed = seed % 10000;       // keep in 4-digit range

    otp[0] = '0' + (seed / 1000);
    otp[1] = '0' + ((seed / 100) % 10);
    otp[2] = '0' + ((seed / 10) % 10);
    otp[3] = '0' + (seed % 10);
    otp[4] = '\0';
}

/*-----------------------------------------------------
   SEND OTP USING GSM
  -----------------------------------------------------*/
void Send_OTP_SMS(u8 *otp)
{
    UART0_TxString("AT\r\n");
    delay_ms(50);

    UART0_TxString("ATE0\r\n");
    delay_ms(50);

    UART0_TxString("AT+CMGF=1\r\n");
    delay_ms(50);

    UART0_TxString("AT+CMGS=\"1234567890\"\r\n");
    delay_ms(50);

    UART0_TxString("Your OTP is: ");
    UART0_TxString((char*)otp);
    delay_ms(50);

    UART0_TxChar(0x1A);
    delay_ms(200);
}

/*-----------------------------------------------------
   GET OTP FROM KEYPAD
  -----------------------------------------------------*/
void Get_OTP_FromKeypad(u8 *buf)
{
    u8 key;
    u8 i = 0;

    CmdLCD(CLEAR_LCD);
    StrLCD("Enter OTP");
    CmdLCD(0xC0);

    while(i < 4)
    {
        key = KeyScan();
        if(key != '\0')
        {
            buf[i++] = key;
            CharLCD('*');
            delay_ms(250);
        }
    }
    buf[4] = '\0';
}

/*-----------------------------------------------------
   SYSTEM INIT
  -----------------------------------------------------*/
void System_Init(void)
{
    UART1_Init();
    UART0_Init();
    Init_LCD();
    Init_KPM();
    i2c_init();

    PINSEL1 &= ~(3 << 18);      // Clear bits for P0.25 (PINSEL1 bits 18–19)

    /* Set P0.25 as OUTPUT */
    IODIR0 |= (1 << 25);

    /* Write default RFID tag (8 bytes) */
    i2c_eeprom_pagewrite(EEPROM_ADDR, 0x00, (u8*)"12345678", 8);
    delay_ms(10);

    /* Default password */
    i2c_eeprom_pagewrite(EEPROM_ADDR, 0x20, (u8*)"12341234", 8);
    delay_ms(10);
}

/*-----------------------------------------------------
   RFID READ (uses interrupt-driven buffer)
  -----------------------------------------------------*/
void RFID_ReadTag(u8 *tag)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("Show RFID Tag");

    /* Reset state before waiting for a new tag */
    r_flag = 0;
    r_index = 0;

    UART1_TxString((unsigned char *)"Waiting for RFID...\r\n");

    /* Wait for ISR to set r_flag == 2 (tag complete).
       This is blocking — you can replace with timeout if needed. */
    while(r_flag != 2) { /* idle */ }

    /* Copy received tag to provided buffer */
    strcpy((char*)tag, (char*)rfid_buff);

    /* Display on LCD */
    CmdLCD(CLEAR_LCD);
    StrLCD("RFID:");
    CmdLCD(0xC0);
    StrLCD((s8 *)tag);

    UART1_TxString((unsigned char *)"RFID: ");
    UART1_TxString((unsigned char *)tag);
    UART1_TxString((unsigned char *)"\r\n");
}

/*-----------------------------------------------------
   CHECK RFID TAG MATCH
  -----------------------------------------------------*/
u8 Check(u8 *tag)
{
    u8 stored[9];
    i2c_eeprom_sequentialread(EEPROM_ADDR, 0x00, stored, 8);
    stored[8] = '\0';

    return (strcmp((char*)tag, (char*)stored) == 0);
}

/*-----------------------------------------------------
   VERIFY PASSWORD
  -----------------------------------------------------*/
u8 VerifyPassword()
{
    u8 pass[9], stored[9], key, i=0;

    CmdLCD(CLEAR_LCD);
    StrLCD("Enter Password");
    CmdLCD(0xC0);

    while(i < 8)
    {
        key = KeyScan();
        if(key != '\0')
        {
            pass[i++] = key;
            CharLCD('*');
            delay_ms(250);
        }
    }
    pass[8] = '\0';

    i2c_eeprom_sequentialread(EEPROM_ADDR, 0x20, stored, 8);
    stored[8] = '\0';

    if(strcmp((char*)pass, (char*)stored) == 0)
    {
        CmdLCD(CLEAR_LCD);
        StrLCD("Password OK");
        delay_s(1);
        return 1;
    }

    CmdLCD(CLEAR_LCD);
    StrLCD("Wrong Password");
    delay_s(2);

    return 0;
}

/*-----------------------------------------------------
   OTP with timeout helper
  -----------------------------------------------------*/
u8 Get_OTP_WithTimeout(u8 *buf, u16 timeout_seconds)
{
    u8 key;
    u8 i = 0;
    u16 elapsed = 0;

    CmdLCD(CLEAR_LCD);
    StrLCD("Enter OTP");
    CmdLCD(0xC0);

    while(i < 4)
    {
        key = KeyScan();
        if(key != '\0')
        {
            buf[i++] = key;
            CharLCD('*');
            delay_ms(200);
        }

        delay_ms(100);       // check every 100 ms
        elapsed++;

        if(elapsed >= (timeout_seconds * 10)) // 10 intervals per second
        {
            return 0;   // TIMEOUT
        }
    }

    buf[4] = '\0';
    return 1;   // success
}

u8 VerifyOTP(void)
{
    u8 otp[5];
    u8 entered[5];

    Generate_OTP(otp);

    CmdLCD(CLEAR_LCD);
    StrLCD("Sending OTP...");
    Send_OTP_SMS(otp);
    delay_s(1);

    /* 20-second timeout (changed to 5 seconds * 4 for demonstration earlier).
       Keep as you need. */
    if(!Get_OTP_WithTimeout(entered, 5))
    {
        CmdLCD(CLEAR_LCD);
        StrLCD("TIMEOUT....!");
        CmdLCD(GOTO_LINE2_POS0+3);
        StrLCD("ACESS DENIED");
        delay_s(2);
        return 0;
    }

    if(strcmp((char*)otp, (char*)entered) == 0)
    {
        CmdLCD(CLEAR_LCD);
        StrLCD("OTP Verified");
        delay_s(1);
        return 1;
    }

    CmdLCD(CLEAR_LCD);
    StrLCD("Wrong OTP");
    delay_s(2);

    return 0;
}

void GrantAccess(void)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("ACCESS GRANTED");
    CmdLCD(0xC0);
    StrLCD("Welcome EMP1");
    delay_s(2);
}

void ShowInvalidCard(u8 *tag)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("INVALID CARD");
    CmdLCD(0xC0);
    StrLCD((s8*)tag);
    delay_s(2);
}

void LED_ON(void)
{
    IOSET0 = (1 << 25);
    delay_s(2);
    IOCLR0 = (1 << 25);
}


