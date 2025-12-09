
/*
#ifndef UART1_H
#define UART1_H

#include <LPC214x.h>

void UART1_Init(void);
void UART1_TxChar(unsigned char ch);
unsigned char UART1_RxChar(void);
void UART1_TxString(unsigned char *str);
void UART1_RxString(unsigned char *str, unsigned int maxlen);

#endif
*/

#ifndef UART1_H
#define UART1_H

#include <LPC214x.h>

/* Externs for ISR <-> application shared variables (defined in system.c) */
extern volatile unsigned char rfid_buff[];   /* buffer for received RFID */
extern volatile unsigned char r_index;      /* current index while receiving */
extern volatile unsigned char r_flag;       /* 0=no,1=receiving,2=ready */
extern volatile unsigned char rch;          /* temporary char read by ISR */

/* UART1 API */
void UART1_Init(void);
void UART1_TxChar(unsigned char ch);
unsigned char UART1_RxChar(void);
void UART1_TxString(unsigned char *str);
void UART1_RxString(unsigned char *str, unsigned int maxlen);

/* ISR prototype (used when linking/setting VIC slot) */
void UART1_isr(void) __irq;

#endif /* UART1_H */
