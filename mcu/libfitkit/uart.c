/*******************************************************************************
   uart.c:
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$

   Popis:
     UART0 drive.
*******************************************************************************/


#include <signal.h>
#include "arch_specific.h"
#include "define.h"
#include "timer_b.h"
#include "globfun.h"
#include "uart.h"
#include "isr_compat.h"

volatile unsigned char TransmitBuf[LengthBuffers_Tx]; ///< buffer pro vysílání
volatile unsigned char TransmitIndex;     ///< index bytu v bufferu pøi zápisu do bufferu
volatile unsigned char TransmitIndexSend; ///< index bytu v bufferu pøi ètení z bufferu pro vysílání
volatile unsigned char TransmitNum;       ///< kolik bytu je v bufferu

volatile unsigned char ReceiveBuf[LengthBuffers_Rx];  ///< buffer pro pøíjem
volatile unsigned int ReceiveIndex;      ///< index bytu v bufferu pri ètení z bufferu
volatile unsigned int ReceiveIndexRec;   ///< index bytu v bufferu pri zápisu do bufferu
volatile unsigned int ReceiveNum;        ///< kolik bytu je v bufferu

volatile unsigned char BreakFlag;         ///< pøíznak BREAKu
volatile unsigned char ErrorFlag;         ///< pøíznak chyby
volatile unsigned char PomRS232;


/**
 \brief Inicializace UART0.
 \param BaudRate - nastavení rychlosti UARTU0
 **/
void serial_init(void) {
  // inicializace promennych
  TransmitIndex     = 0;
  TransmitIndexSend = 0;
  TransmitNum       = 0;
  ReceiveIndex      = 0;
  ReceiveIndexRec   = 0;
  ReceiveNum        = 0;
  BreakFlag         = 0;
  ErrorFlag         = 0;

  #if defined MSP_16X
    // nastaveni serioveho kanalu (FITkit 1.x)
    U0CTL |= SWRST;              // Reset USART1
    U0CTL  = SWRST | CHAR;       // 8-bit character, no parity, 1 stop bit
    U0TCTL = SSEL_2;             // BRCLK = SMCLK (7.3728MHz)
    U0RCTL = URXEIE;             // prijem vsech znaku i chybnych
    U0BR0  = UART_BR0;           //nastaveni rychlosti (viz arch_specific.h)
    U0BR1  = UART_BR1;
    
    U0MCTL = 0x00;                        // no modulation
    ME1   |= UTXE0 + URXE0;               // Enabled USART0 RXD a TXD

    P3SEL |= TXDATA + RXDATA;             // P3.4 a P3.5 UART0
    P3DIR |= TXDATA;                      // TXDATA vystup
    P3DIR &= ~RXDATA;                     // RXDATA vstup

    U0CTL &= ~SWRST;                      // Povoleni USART
    IE1   |= URXIE0;                      // povoleni preruseni pri prijmu

  #elif defined MSP_261X
    // nastaveni serioveho kanalu (FITkit 2.x)

    UCA0CTL1 |= UCSWRST;                   // Reset USCI A0
    UCA0CTL0 = UCMODE_0 ;                  // 8-bit character, no parity, 1 stop bit
    UCA0CTL1 = UCSWRST | UCSSEL_2;         // BRCLK = SMCLK (7.3728MHz)
    UCA0BR0 = UART_BR0;                    //nastaveni rychlosti (viz arch_specific.h)
    UCA0BR1 = UART_BR1;
    UCA0MCTL = 0x00;                       // bez modulace

    P3SEL |= TXDATA + RXDATA;               // P3.4,5 = USCI_A0 TXD/RXD
    P3DIR |= TXDATA;                        // TXDATA vystup
    P3DIR &= ~RXDATA;                      // RXDATA vstup

    UCA0CTL1 &= ~UCSWRST;

    IE2 |= UCA0RXIE;                       // povoleni preruseni pri prijmu
    IE2 &= ~UCA0TXIE;                      // zakaz preruseni pri vysilani
  #else
    #error "Can't initialize UART"
  #endif
}

/**
 \brief Deinicializace UART0.
 **/
void serial_close(void) {
  #if defined MSP_16X
     IE1   &= ~(URXIE0 | UTXIE0);          // zakaz preruseni pri prijmu a vysilani
     ME1   &= ~(UTXE0 | URXE0);            // Disable USART0 RXD a TXD
     U0CTL |= SWRST;                       // Reset USART1
  #elif defined MSP_261X
     IE2 &= ~(UCA0RXIE | UCA0TXIE);         // zakaz preruseni pri prijmu/vysilani
     UCA0CTL1 |= UCSWRST;                   // Reset USCI A0
  #endif

  // inicializace promennych, vyprazdneni buffru
  TransmitIndex     = 0;
  TransmitIndexSend = 0;
  TransmitNum       = 0;
  ReceiveIndex      = 0;
  ReceiveIndexRec   = 0;
  ReceiveNum        = 0;
  BreakFlag         = 0;
}


/**
 \brief Odeslání znaku.
 \param Uchar - vysílaný znak
 **/
void term_send_char(char Uchar) {

  while (TransmitNum >= LengthBuffers_Tx) WDG_reset();

  TransmitBuf[TransmitIndex] = Uchar;
  TransmitIndex++;
  if (TransmitIndex >= LengthBuffers_Tx)
     TransmitIndex = 0;
  TransmitNum++;

  UART_enable_tx_irq(); // povoleni preruseni na vysilani
}


/**
 \brief Odeslání znaku bez pøerušení v idle.
 \param Uchar - vysílaný znak
 **/
void term_send_char_idle(unsigned char Uchar) {
  while (UART_TX_BUF_FULL) WDG_reset();
  UART_TX_BUF = Uchar;
}


/**
 \brief Odeslání znaku v hexa.
 \param Uchar - vysílaný znak
 **/
void term_send_char_hex(unsigned char Uchar) {
  unsigned char buf[3];

  chr2hex((unsigned char *)&buf, Uchar);

  term_send_char(buf[0]);
  term_send_char(buf[1]);
}


/**
 \brief Odeslání znaku CRLF.
 **/
/* -- makro
void term_send_crlf(void) {
  term_send_char(CR);
  term_send_char(LF);
}
*/

/**
 \brief Odeslání øetìzce.
     - Délka øetìzce nesmí pøesáhnout velikost výstupního bufferu (LengthBuffers).
 \param ptrStr - vysilany øetìzec
 **/
void term_send_str(char *ptrStr) {
  unsigned char Index = 0;

  while (ptrStr[Index] != 0) {

    WDG_reset();                                // software watchdog

    term_send_char(ptrStr[Index]);
    Index++;
  }
}

/**
 \brief Odeslání øetìzce. Na konci doplnìno o CR a LF
     - Délka øetìzce nesmí pøesáhnout velikost výstupního bufferu (LengthBuffers).
 \param ptrStr - vysílaný øetìzec
 **/
/* -- makro
void term__snd_str_CRLF(char *ptrStr) {
  term_send_str(ptrStr);
  term_send_crlf();
}
*/

/**
 \brief Odeslání celého znaménkového èísla v rozsahu 'int'.
 \param iNum - odesílané èíslo
 **/
void term_send_num(long int iNum) {
  unsigned char buf[20];

  long2str(iNum, (unsigned char *)&buf, 10);
  term_send_str((unsigned char *)&buf);
}


/**
 \brief Odeslání realného èísla.
 \param intn - celá èást
 \param desn - desetinná èást
 **/
/* -- makro
void term_snd_real(long intn, long desn) {
  term_send_num(intn);
  term_send_char('.');
  term_send_num(desn);
}
*/


/**
 \brief Odeslání celého znaménkového èísla v rozsahu 'int' v hexa.
 \param iNum - odesílané èíslo
 **/
void term_send_hex(long int iNum) {
  unsigned char buf[20];

  long2str(iNum, (unsigned char *)&buf, 16);
  term_send_str((unsigned char *)&buf);
}


/**
 \brief Ètení znaku ze sériové linky.
 \return pøijatý znak (8 bitu)
 **/
unsigned char term_rcv_char(void) {
  unsigned char Uchar;
  Uchar = 0;
  if (ReceiveNum > 0) {
    Uchar = ReceiveBuf[ReceiveIndex];
    ReceiveIndex++;
    if (ReceiveIndex >= LengthBuffers_Rx)
      ReceiveIndex = 0;
    ReceiveNum--;
  }
  return Uchar;
}


/**
 \brief Ètení bloku dat ze seriové linky.
 \param out - výstupní buffer
 \param count - maximální délka
 \return -1 - pøi chybì
       \n jinak délka pøijatých dat
 **/
signed int term_rcv_block(unsigned char *out, int count) {
  int used = 0;
  unsigned char err_cnt = 127;
  while ((used < count) && (err_cnt > 0)) {
    if (ReceiveNum == 0) {

      WDG_reset();

      delay_ms(1);
      err_cnt--;
    }
    else {
      err_cnt = 127;
      *(out + used++) = ReceiveBuf[ReceiveIndex];
      if (ReceiveIndex == LengthBuffers_Rx-1)
        ReceiveIndex = 0;
      else
        ReceiveIndex++;
      ReceiveNum--;
    }
  }
  if (err_cnt == 0)
    return -1;
  else
    return used;
}

/**
 \brief Detekce odeslání rámce.
 \return  1 - ještì se vysílá
      \n  0 - data jsou odeslána
 **/
/* -- makro
unsigned char term_snd_flag(void) {
  if ((TXEPT & U0TCTL) != 0)
    return 0;
  else
    return 1;
  //return (TXEPT & U0TCTL) ? 0 : 1;
}
*/

/**
 \brief Test pøijatých dat.
 \return kolik dat je v pøijímacím bufferu
 **/
/* -- makro
unsigned int term_rcv_flag(void) {
  return ReceiveNum;
}
*/

/**
 \brief Fce pøijme jeden znak ze vstupu v maximalním èase timeout (v msec).
 \param timeout - èasový limit v ms
 \return -1 - pøi chybì 
    \n   jinak se jedná o pøijatý znak    
 **/
int term_rcv_char_t(long int timeout) {
  unsigned char Flag;

  Flag = term_rcv_flag();           // pocet bytu ve vyrovnacim bufferu RS232

  // dokud neprijde znak, nebo nevyprsi timeout mezi dvema znaky, nebo neprijde break
  while (Flag == 0) {
    if (--timeout < 0)
      return (-1);                      // vyprsel timeout mezi dvema znaky.

    WDG_reset();                                        // SW watchdog

    delay_ms(1);                         // cekam 1ms
    Flag = term_rcv_flag();         // pocet bytu ve vyrovnacim bufferu RS232
  }
  return term_rcv_char();
}

/**
 \brief Fce vrací poèet volných bytu ve vysílacím bufferu.
 \return Poèet volných bytù ve vysílacím bufferu
 **/
/* -- makro
unsigned char term_trnsmt_flag(void) {
  return (LengthBuffers_Tx - TransmitNum);
}
*/


/**
   \brief Fce vyprázdní pøijímací buffer.
 **/
/* -- makro
void term_flush_rx_buf(void) {
  while (term_rcv_flag() != 0) {
    term_rcv_char();
  }
}
*/

/**
   \brief Fce vyprázdní vysílací buffer.
 **/
/* -- makro
void term_flush_tx_buf(void) {
  while ((TransmitNum != 0) || ((U0TCTL & TXEPT) == 0));
}
*/

/**
   \brief Detekce BREAKu.
   \return 0 - nenastal BREAK
        \n 1 - BREAK nastal
 **/
/* -- makro
unsigned char term_dtct_break(void) {
  
  //return (BreakFlag != 0)? 1:0;
  
  return (BreakFlag != 0);
}
*/


/**
   \brief Nuluje pøíznak indikující BREAK.
 **/
/* -- makro 
void term_rst_break_flag(void) {
  BreakFlag = 0;
}
*/

/**
   \brief Detekce ERROR.
   \return 0 - nenastal ERROR,
        \n 1 - nastal ERROR
 **/
/* -- makro
unsigned char term_dtct_error(void) {
  // tak tohle nechapu, asi jsem debil
  unsigned char Errf;
  if (ErrorFlag != 0) {
    ErrorFlag = 0;
    return Errf;
  }
  else
    return 0;
}
*/

/**
 \brief  Preruseni pro vysilani dat.
 **/
#if defined MSP_16X
ISR(USART0TX, TxintrRS232) {      
#elif defined MSP_261X
ISR(USCIAB0TX, TxintrRS232) {      
#endif
  if (TransmitNum > 0) {
    UART_TX_BUF = TransmitBuf[TransmitIndexSend];   // vyslani bytu
    TransmitNum--;
    TransmitIndexSend++;
    if (TransmitIndexSend >= LengthBuffers_Tx)
      TransmitIndexSend = 0;
  }
  if (TransmitNum == 0)
     UART_disable_tx_irq();                      // je-li posledni byte, zakaz preruseni
}

/**
 \brief Preruseni pri prijmu dat.
 **/
#if defined MSP_16X
ISR(USART0RX, RxintrRS232) {      
#elif defined MSP_261X
ISR(USCIAB0RX, RxintrRS232) {      
#endif
#ifdef MSP_16X
  // detekce BREAKu, musi byt pred ctenim UART_RX_BUF
  BreakFlag = ((U0RCTL & 0x10) != 0);
  if ((U0RCTL & RXERR) != 0) {                   // prijem chybneho znaku
    ErrorFlag = U0RCTL & 0xF0;
    PomRS232 = UART_RX_BUF;
  }
  else {
#endif	 
    if (ReceiveNum < LengthBuffers_Rx) {
      ReceiveBuf[ReceiveIndexRec] = UART_RX_BUF;  // zapis dat do buffru
      ReceiveIndexRec++;
      if (ReceiveIndexRec >= LengthBuffers_Rx)
        ReceiveIndexRec = 0;
      ReceiveNum++;
    }
#ifdef MSP_16X
  }
#endif
}
