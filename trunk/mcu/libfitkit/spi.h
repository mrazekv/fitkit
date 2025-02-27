/*******************************************************************************
   spi.h:
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
     Sdileny ovladac pro flash pamet and FPGA SpartanIII.
*******************************************************************************/


#ifndef _SPI_H_
#define _SPI_H_

// SPI rozhrani pro komunikaci s FPGA a externi FLASH
#define SPI_CSn     BIT0        ///< obecný výstup jako chip select, aktivní v 0
#define SPI_DO      BIT1        ///< datový výstup v master módu
#define SPI_DI      BIT2        ///< datový vstup v master módu
#define SPI_CLK     BIT3        ///< hodinový výstup v master módu
#define DONE        BIT7        ///< DONE vstup do FPGA
#define INITn       BIT4        ///< INIT vstup/výstup FPGA, aktivní v 0
#define PROGn       BIT6        ///< PROG vstup do FPGA, aktivní v 0

#define SPI_PORT_DIR   P5DIR       ///< direct port
#define SPI_PORT_IN    P5IN        ///< vstupní port
#define SPI_PORT_OUT   P5OUT       ///< výstupní port
#define SPI_PORT_SEL   P5SEL       ///< selekèní port

#if defined MSP_16X
 #define SPI_RX_BUF U1RXBUF
 #define SPI_TX_BUF U1TXBUF
// #define SPI_TX_BUF_FULL ((TXEPT & U1TCTL) == 0)
 #define SPI_BUSY ((TXEPT & U1TCTL) == 0)
#elif defined MSP_261X
 #define SPI_RX_BUF UCB1RXBUF
 #define SPI_TX_BUF UCB1TXBUF
// #define SPI_TX_BUF_FULL ((UC1IFG & UCB1TXIFG) == 0)
 #define SPI_BUSY ((UCBUSY & UCB1STAT) != 0)
// #define SPI_BUSY (!(UC1IFG & UCB1TXIFG))
#else 
 #error "SPI define"
#endif

/**
 \brief Inicializace SPI
 **/
void SPI_Init(void);

/**
 \brief Deaktivace SPI
 **/
void SPI_Close(void);

inline unsigned char SPI_read_wait_write(unsigned char datain);

inline unsigned char SPI_write_wait_read(unsigned char datain);

/**
 \brief Zápis/ètení do/z flash pamìti
 \param WrByte - zapisovaný byte
 \return pøeètený byte
 **/
#define SPI_read_write SPI_write_wait_read


/**
 \brief Výbìr SPI FLASH pameti, aktivní v 0
 \param active - 1 - aktivni CS
                  \n jinak neaktivni
 **/
#define SPI_set_cs_FLASH(active) \
  {  SPI_PORT_OUT = ((active) == 0) ? (SPI_PORT_OUT | SPI_CSn) : (SPI_PORT_OUT & ~SPI_CSn); }


/**
 \brief Výbìr SPI FPGA, aktivní v 0
 \param active - 1 - aktivni CS
                  \n jinak neaktivni
 **/
#define SPI_set_cs_FPGA(active) \
   { SPI_PORT_OUT = ((active) == 0) ? (SPI_PORT_OUT | BIT4) : (SPI_PORT_OUT & ~BIT4); }


/**
 \brief Zápis do flash pamìti
 \param data - zapisovaný byte
 **/
#define SPI_write(data) \
  { SPI_TX_BUF = (data); \
    while (SPI_BUSY) WDG_reset(); /*cekat dokud neni vysilaci buffer prazdny*/ \
  }

#endif /* _SPI_H_ */
