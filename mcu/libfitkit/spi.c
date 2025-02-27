/*******************************************************************************
   spi.c:
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
     Funkce pro obsluhu SPI
*******************************************************************************/


#include "arch_specific.h"
#include "define.h"
#include "spi.h"

/**
 \brief Inicializace SPI
 **/
void SPI_Init(void) {
  #if defined MSP_16X
    // nastaveni SPI na UART1 (FITkit 1.x)

    U1CTL |= SWRST;                       /// uvest SPI do resetu
    U1CTL = SWRST | CHAR | SYNC | MM;     /// SPI mode, 8 bitu, master mode
    U1TCTL = CKPL | SSEL_3 | STC;         /// SMCLK clock (7MHz), idle polarita na 1
    U1BR0 = 2;                            /// rychlost na maximum (SMCLK/2)
    U1BR1 = 0;
    ME2   |= USPIE1;                      /// povoleni SPI

    // nastaveni vstup/vystupu
    SPI_PORT_DIR &= ~(SPI_DI);               // vstupy
    SPI_PORT_DIR |= SPI_CSn | INITn | SPI_DO | SPI_CLK;       //  vystupy
    SPI_PORT_OUT |= SPI_CSn | INITn | SPI_DO | SPI_CLK;       // vystupy na 1

    SPI_PORT_SEL |= SPI_DI | SPI_DO | SPI_CLK;        // pripojeni pinu na USART

    U1CTL     &= ~SWRST;                  // povoleni SPI (zruseni resetu)
  #elif defined MSP_261X
    // nastaveni SPI na UCB1 (FITkit 2.x)
  
    UCB1CTL1 = UCSWRST;                   /// Reset UCB1

    UCB1CTL0 = UCCKPL | UCMST | UCMSB | UCMODE_0 | UCSYNC; /// SPI mode, 8 bitu, master mode, idle polarita na 1
    UCB1CTL1 |= UCSSEL_2;         /// SMCLK clock
    UCB1BR0 = 2;                  /// rychlost SMCLK/2 (teoreticky muze jet i na SMCLK/1, ale je treba zvysit frekv. v FPGA)
    UCB1BR1 = 0;

    // nastaveni vstup/vystupu
    SPI_PORT_DIR &= ~(SPI_DI);                   // vstupy
    SPI_PORT_DIR |= SPI_CSn | INITn | SPI_DO | SPI_CLK;  // vystupy
    SPI_PORT_OUT |= SPI_CSn | INITn | SPI_DO | SPI_CLK;  // vystupy na 1

    SPI_PORT_SEL |= SPI_DI | SPI_DO | SPI_CLK;   // pripojeni pinu na UCB1

    //UC1IFG = 0;
    UC1IE &= ~(UCB1TXIE | UCB1RXIE);

    UCB1CTL1 &= ~UCSWRST;                     // odresetovani UCB1
  #else 
    #error "Can't initialize SPI"
  #endif
}

/**
 \brief Deaktivace SPI
 **/
void SPI_Close(void) {
  #if defined MSP_16X
    // deaktivace SPI
    ME2  &= ~USPIE1;                      // povoleni SPI
    U1CTL =  SWRST;                       // zakaz (reset) SPI
  #elif defined MSP_261X
    UCB1CTL1 |= UCSWRST;                   /// Reset UCB1
  #endif

  // deaktivace vstupu/vystupu
  SPI_PORT_SEL &= ~(SPI_DI | SPI_DO | SPI_CLK);     // odpojeni pinu na USART1 SPI
  SPI_PORT_DIR &= ~(SPI_DI | SPI_DO | SPI_CLK | SPI_CSn);   // vsechno vstupy
}

/**
 \brief Zápis/ètení pøes SPI do/z flash pamìti / FPGA
 \param datain - hodnota, která bude zapsána pøes SPI
 \return pøeètený byte
 **/
inline unsigned char SPI_read_wait_write(unsigned char datain) {
  unsigned char result;  /* nacteni dat */ 
  while (SPI_BUSY) WDG_reset();
  result = SPI_RX_BUF; 
  SPI_TX_BUF = datain;  /* vyslani datoveho byte pres SPI  */
  return result;
}


/**
 \brief Zápis/ètení pøes SPI do/z flash pamìti / FPGA
 \param datain - hodnota, která bude zapsána pøes SPI
 \return pøeètený byte
 **/
inline unsigned char SPI_write_wait_read(unsigned char datain) {
  SPI_TX_BUF = datain;  /* vyslani datoveho byte pres SPI  */

  while (SPI_BUSY) WDG_reset();
  //  while (SPI_TX_BUF_FULL) WDG_reset(); /* cekani, az je odvysilano */
  
  return SPI_RX_BUF; /* precteni prijate hodnoty */
}
