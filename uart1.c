
/*
//uart1.c
#include <LPC214x.h>
#include "uart1.h"

// UART1 Configuration
#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)
#define BAUD 9600
#define DIVISOR (PCLK/(16*BAUD))

#define RDR_BIT 0
#define THRE_BIT 5
#define TEMT_BIT 6

void UART1_Init(void)
{
    // P0.8 = TXD1 , P0.9 = RXD1
    PINSEL0 |= (1<<16) | (1<<18);

    U1LCR = 0x83;      // 8-bit, enable DLAB
    U1DLL = DIVISOR;
    U1DLM = DIVISOR >> 8;
    U1LCR = 0x03;      // Disable DLAB
}

void UART1_TxChar(unsigned char ch)
{
    U1THR = ch;
    while(((U1LSR >> TEMT_BIT) & 1) == 0);
}

unsigned char UART1_RxChar(void)
{
    while(((U1LSR >> RDR_BIT) & 1) == 0);
    return U1RBR;
}

void UART1_TxString(unsigned char *str)
{
    while(*str)
        UART1_TxChar(*str++);
}

void UART1_RxString(unsigned char *str, unsigned int maxlen)
{
    unsigned int i=0;
    unsigned char ch;

    while(1)
    {
        ch = UART1_RxChar();

        if(ch=='\r' || ch=='\n')
        {
            str[i] = '\0';
            break;
        }
        else if(i >= maxlen-1)
        {
            str[i] = '\0';
            break;
        }
        else
        {
            str[i++] = ch;
        }
    }
}
*/
/*

#include <LPC214x.h>
#include "uart1.h"

// UART1 Configuration 
#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)
#define BAUD 9600
#define DIVISOR (PCLK/(16*BAUD))

// bit positions 
#define RDR_BIT 0
#define THRE_BIT 5
#define TEMT_BIT 6

void UART1_Init(void)
{
    // P0.8 = TXD1 , P0.9 = RXD1 
    PINSEL0 |= (1<<16) | (1<<18);

    U1LCR = 0x83;                       
    U1DLL = DIVISOR & 0xFF;
    U1DLM = (DIVISOR >> 8) & 0xFF;
    U1LCR = 0x03;                      

    // Enable UART1 RX interrupt 
    U1IER = 0x01;                       

    //Configure VIC to use UART1_isr 
    VICIntSelect &= ~(1 << 7);          
    VICVectAddr1 = (unsigned)UART1_isr; 
    VICVectCntl1 = 0x20 | 7;            
    VICIntEnable |= (1 << 7);           
}

void UART1_TxChar(unsigned char ch)
{
    U1THR = ch;
   
    while(((U1LSR >> TEMT_BIT) & 1) == 0);
}

unsigned char UART1_RxChar(void)
{
    while(((U1LSR >> RDR_BIT) & 1) == 0);
    return U1RBR;
}

void UART1_TxString(unsigned char *str)
{
    while(*str)
        UART1_TxChar(*str++);
}

void UART1_RxString(unsigned char *str, unsigned int maxlen)
{
    unsigned int i=0;
    unsigned char ch;

    while(1)
    {
        ch = UART1_RxChar();

        if(ch=='\r' || ch=='\n')
        {
            str[i] = '\0';
            break;
        }
        else if(i >= maxlen-1)
        {
            str[i] = '\0';
            break;
        }
        else
        {
            str[i++] = ch;
        }
    }
}


void UART1_isr(void) __irq
{
    unsigned int iid = U1IIR & 0x0E; 
   
    if ((U1IIR & 0x04) || (iid == 0x04))
    {
        rch = U1RBR;   

        if (rch == 0x02)          \
        {
            r_index = 0;
            r_flag = 1;           
        }
        else if (r_flag == 1 && rch != 0x03) 
        {
            if (r_index < 19)               
                rfid_buff[r_index++] = rch;
        }
        else if (rch == 0x03)      
        {
            rfid_buff[r_index] = '\0';
            r_flag = 2;            
        }
    }

   
    VICVectAddr = 0; 
}
*/

#include <LPC214x.h>
#include "uart1.h"

/* UART1 Configuration */
#define FOSC   12000000
#define CCLK   (FOSC*5)
#define PCLK   (CCLK/4)
#define BAUD   9600
#define DIVISOR (PCLK/(16*BAUD))

/* ---------------- UART1 INITIALIZATION ---------------- */
void UART1_Init(void)
{
    // P0.8 = TXD1 , P0.9 = RXD1
    PINSEL0 |= (1<<16) | (1<<18);

    U1LCR = 0x83;                // 8-bit, DLAB = 1
    U1DLL = DIVISOR & 0xFF;
    U1DLM = (DIVISOR >> 8) & 0xFF;
    U1LCR = 0x03;                // DLAB = 0

    U1IER = 0x01;                // Enable RX interrupt

    // Configure VIC for UART1 interrupt
    VICIntSelect &= ~(1 << 7);          // IRQ
    VICVectAddr1 = (unsigned)UART1_isr; // ISR address
    VICVectCntl1 = 0x20 | 7;            // Enable slot, UART1 source
    VICIntEnable |= (1 << 7);           // Enable UART1 interrupt
}

/* ---------------- UART1 TX FUNCTIONS ---------------- */
void UART1_TxChar(unsigned char ch)
{
    U1THR = ch;
    while(!(U1LSR & (1<<6)));     // Wait for transmitter empty
}

void UART1_TxString(unsigned char *str)
{
    while(*str)
        UART1_TxChar(*str++);
}

/* ---------------- UART1 ISR (RFID READER) ---------------- */
/* NOTE: Globals for ISR are defined in system.c */
void UART1_isr(void) __irq
{
    if (U1IIR & 0x04)             // RX interrupt
    {
        rch = U1RBR;              // Read byte

        if (rch == 0x02)          // STX
        {
            r_index = 0;
            r_flag  = 1;
        }
        else if (r_flag == 1 && rch != 0x03)
        {
            if (r_index < 19)
                rfid_buff[r_index++] = rch;
        }
        else if (rch == 0x03)     // ETX
        {
            rfid_buff[r_index] = '\0';
            r_flag = 2;           // Tag ready
        }
    }

    VICVectAddr = 0;              // End ISR
}

