/*******************************************************************************
   uart.h:
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

#ifndef _UART_H_
#define _UART_H_

#define LengthBuffers_Rx 127              ///< d�lka vyrovn�vac�ho Rx bufferu
#define LengthBuffers_Tx 100              ///< d�lka vyrovn�vac�ho Tx bufferu

#define TXDATA  BIT4                      ///< sign�l TXDATA
#define RXDATA  BIT5                      ///< sign�l RXDATA

#ifdef MSP_16X
 #define UART_RX_BUF RXBUF0
 #define UART_TX_BUF TXBUF0
 #define UART_TX_BUF_FULL ((IFG1 & UTXIFG0) == 0)
 #define UART_RX_BUF_EMPTY ((IFG1 & URXIFG0) == 0)

 #define UART_enable_tx_irq() {IE1 |= UTXIE0;}
 #define UART_disable_tx_irq() {IE1 &= ~UTXIE0;}

#endif
#ifdef MSP_261X
 #define UART_RX_BUF UCA0RXBUF
 #define UART_TX_BUF UCA0TXBUF
 #define UART_TX_BUF_FULL ((IFG2 & UCA0TXIFG) == 0)
 #define UART_RX_BUF_EMPTY ((IFG2 & UCA0RXIFG) == 0)

 #define UART_enable_tx_irq() {IE2 |= UCA0TXIE;}
 #define UART_disable_tx_irq() {IE2 &= ~UCA0TXIE;}
#endif

#define serial_tx(ach) \
 {while (UART_TX_BUF_FULL) WDG_reset(); \
 UART_TX_BUF = (ach);}

#define serial_rx(ach) \
 {while (UART_RX_BUF_EMPTY) WDG_reset(); \
 ach = UART_RX_BUF; }

/**
 \brief Inicializace seriov�ho kan�lu.
 **/
void serial_init(void);

/**
 \brief Deinicializace seriov�ho kan�lu.
 **/
void serial_close(void);


/**
 \brief Odesl�n� znaku.
 \param Uchar - vys�lan� znak
 **/
void term_send_char(char Uchar);

/**
 \brief Odesl�n� znaku bez p�eru�en� v idle.
 \param Uchar - vys�lan� znak
 **/
void term_send_char_idle(unsigned char Uchar);

/**
 \brief Odesl�n� znaku v hexa.
 \param Uchar - vys�lan� znak
 **/
void term_send_char_hex(unsigned char Uchar);


#define term_send_bool(cond) term_send_char((cond) ? '1' : '0')
/**
 \brief Odesl�n� znaku CRLF.
 **/
/*
void term_send_crlf(void);
*/
#define term_send_crlf() { term_send_char(CR); term_send_char(LF); }

/**
 \brief Odesl�n� �et�zce.
     - D�lka �et�zce nesm� p�es�hnout velikost v�stupn�ho bufferu (LengthBuffers).
 \param ptrStr - vysilany �et�zec
 **/
void term_send_str(char *ptrStr);

/**
 \brief Odesl�n� �et�zce. Na konci dopln�no o CR a LF
     - D�lka �et�zce nesm� p�es�hnout velikost v�stupn�ho bufferu (LengthBuffers).
 \param ptrStr - vys�lan� �et�zec
 **/
/*
void term_send_str_crlf(char *ptrStr);
*/
#define term_send_str_crlf(ptrStr) { term_send_str(ptrStr); term_send_crlf(); }

/**
 \brief Odesl�n� cel�ho znam�nkov�ho ��sla v rozsahu 'int'.
 \param iNum - odes�lan� ��slo
 **/
void term_send_num(long int iNum);

/**
 \brief Odesl�n� realn�ho ��sla.
 \param intn - cel� ��st
 \param desn - desetinn� ��st
 **/
/*
void term_snd_real(long intn, long desn);
*/
//#define term_snd_real(inum, fnum) { term_send_num(inum); term_send_char('.'); term_send_num(fnum); }

/**
 \brief Odesl�n� cel�ho znam�nkov�ho ��sla v rozsahu 'int' v hexa.
 \param iNum - odes�lan� ��slo
 **/
void term_send_hex(long int iNum);

/**
 \brief �ten� znaku ze s�riov� linky.
 \return p�ijat� znak (8 bitu)
 **/
unsigned char term_rcv_char(void);

/**
 \brief �ten� bloku dat ze s�riov� linky.
 \param out - v�stupn� buffer
 \param count - maxim�ln� d�lka
 \return -1 - p�i chyb�
       \n jinak d�lka p�ijat�ch dat
 **/
signed int term_rcv_block(unsigned char *out, int count);

/**
 \brief Detekce odesl�n� r�mce.
 \return  1 - je�t� se vys�l�
      \n  0 - data jsou odesl�na
 **/
/*
unsigned char term_snd_flag(void);
*/
#define term_snd_flag() (unsigned char)((TXEPT & U0TCTL) ? 0 : 1)

/**
 \brief Test p�ijat�ch dat.
 \return kolik dat je v p�ij�mac�m bufferu
 **/
/*
unsigned int term_rcv_flag(void);
*/
extern volatile unsigned int ReceiveNum;
#define term_rcv_flag() (unsigned int)(ReceiveNum)

/**
 \brief Fce p�ijme jeden znak ze vstupu v maximaln�m �ase timeout (v msec).
 \param timeout - �asov� limit v ms
 \return -1 - p�i chyb� 
    \n   jinak se jedn� o p�ijat� znak    
 **/
int term_rcv_char_t(long int timeout);

/**
 \brief Fce vrac� po�et voln�ch bytu ve vys�lac�m bufferu.
 \return Po�et voln�ch byt� ve vys�lac�m bufferu
 **/
/*
unsigned char term_trnsmt_flag(void);
*/
extern volatile unsigned char TransmitNum;
#define term_trnsmt_flag() (unsigned char)(LengthBuffers_Tx - TransmitNum)

/**
   \brief Fce vypr�zdn� p�ij�mac� buffer.
 **/
void term_flush_rx_buf(void);
#define term_flush_rx_buf() while (term_rcv_flag() != 0) { term_rcv_char(); }

/**
   \brief Fce vypr�zdn� vys�lac� buffer.
 **/
/*
void term_flush_tx_buf(void);
*/
extern volatile unsigned char TransmitNum;
#define term_flush_tx_buf() while ((TransmitNum != 0) || ((U0TCTL & TXEPT) == 0));

/**
   \brief Detekce BREAKu.
   \return 0 - nenastal BREAK
        \n 1 - BREAK nastal
 **/
/*
unsigned char term_dtct_break(void);
*/
extern volatile unsigned char BreakFlag;
#define term_dtct_break() (unsigned char)(BreakFlag != 0)

/**
   \brief Nuluje p��znak indikuj�c� BREAK.
 **/
/*
void term_rst_break_flag(void);
*/
extern volatile unsigned char BreakFlag;
#define term_rst_break_flag() { BreakFlag = 0; }

/**
   \brief Detekce ERROR.
   \return 0 - nenastal ERROR,
        \n 1 - nastal ERROR
 **/
/*
unsigned char term_dtct_error(void);
*/
// stoji se za to podivat jak to bylo naprogramovane :)
extern volatile unsigned char ErrorFlag;
#define term_dtct_error() (unsigned char)(ErrorFlag ? ((ErrorFlag = 0), 1) : 0)

/**
 \brief Vyslani ESC sekvence. 
 - Delka retezce nesmi byt vetsi nez velikost vyst. bufferu (LengthBuffers).
 \param ptrStr - vysilany retazec
 **/
#define term_send_esc_seq(ptrStr) { term_send_char(ESC); term_send_str(ptrStr); }

#endif   /* _UART_H_ */
