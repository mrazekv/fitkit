/*******************************************************************************
   fspi.c: second SPI interface
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Karel Slany <slany AT fit.vutbr.cz>

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


*******************************************************************************/

#include "fspi.h"

/**
 \brief SPI initialization
 **/
void FSPI_Init(char smclkdiv)
{
 #if defined MSP_16X
    while(1) {} //FITkit 1.x not supported

  /*
  // set SPI to UART0 (FITkit 1.x)

  U0CTL |= SWRST;                   /// reset SPI
  U0CTL = SWRST | CHAR | SYNC | MM; /// SPI mode, 8 bit, master mode
  U0TCTL = CKPL | SSEL_3 | STC;     /// SMCLK clock (7MHz), idle polarity 1
  U0BR0 = 2;                        /// max speed (SMCLK/2)
  U0BR1 = 0;
  ME2 |= USPIE1;                    /// enable SPI

  // I/O setup
  FSPI_PORT_DIR &= ~(FSPI_DI);         // inputs
  FSPI_PORT_DIR |= FSPI_DO | FSPI_CLK; // outputs
  FSPI_PORT_OUT |= FSPI_DO | FSPI_CLK; // set outputs to 1

  FSPI_PORT_SEL |= FSPI_DI | FSPI_DO | FSPI_CLK; // connects pins to  USART

  U0CTL &= ~SWRST; // enable SPI (disable reset)
  */
 #elif defined MSP_261X
  // set SPI to UCB0 (FITkit 2.x)
 
  UCB0CTL1 = UCSWRST; /// reset UCB0

  UCB0CTL0 = UCCKPH | UCMST | UCMSB | UCMODE_0 | UCSYNC; /// SPI mode, 8 bit, master mode, msb first, idle polarity 0
  UCB0CTL1 |= UCSSEL_2;                                  /// SMCLK clock
  UCB0BR0 = smclkdiv;                                    /// speed SMCLK/smclkdiv = 14/smclkdiv MHz
  UCB0BR1 = 0;

  // I/O setup
  FSPI_PORT_DIR &= ~(FSPI_DI);         // inputs
  FSPI_PORT_DIR |= FSPI_DO | FSPI_CLK | FSPI_CS; // outputs
  FSPI_PORT_OUT &= ~FSPI_CLK; // set outputs to 0
  FSPI_PORT_OUT |= FSPI_DO | FSPI_CS; // set outputs to 1

  FSPI_PORT_SEL |= SPI_DI | SPI_DO | SPI_CLK; // connects pins to UCB0

  //UC1IFG = 0;
  UC0IE &= ~(UCB0TXIE | UCB0RXIE);

  UCB0CTL1 &= ~UCSWRST; // disable UCB0 reset
 #else 
  #error "Can't initialize SPI"
 #endif
}


/**
 \brief Disables SPI
 **/
void FSPI_Close(void)
{
 #if defined MSP_16X
  /*
  // disable SPI
  ME2 &= ~USPIE1; // enable SPI
  U0CTL = SWRST;  // disable (reset) SPI
  */
 #elif defined MSP_261X
  UCB0CTL1 |= UCSWRST; /// reset UCB0
 #endif

  // disable I/O
  FSPI_PORT_SEL &= ~(FSPI_DI | FSPI_DO | FSPI_CLK);                // disconnect pins USART0 SPI
  FSPI_PORT_DIR &= ~(FSPI_DI | FSPI_DO | FSPI_CLK | FSPI_CS); // set all as inputs
}

/**
 \Brief Sends and receives 8bits over SPI
 **/
inline unsigned char FSPI_write_wait_read(unsigned char data)
{
  FSPI_TX_BUF = data;                  
  while (FSPI_BUSY) {WDG_reset();}
  return FSPI_RX_BUF;
}

/**
 \Brief Sends and receives 8bits over SPI
 **/
inline void FSPI_write_wait(unsigned char data)
{
  FSPI_TX_BUF = data;                  
  while (FSPI_BUSY) {WDG_reset();}
}
