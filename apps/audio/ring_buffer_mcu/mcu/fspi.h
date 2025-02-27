/*******************************************************************************
   fspi.h: second SPI interface
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

#ifndef _FSPI_H_
#define _FSPI_H_

#include <fitkitlib.h>

// fast SPI interface connected to FPGA
#define FSPI_DO BIT1        ///< master DO
#define FSPI_DI BIT2        ///< master DI
#define FSPI_CLK BIT3       ///< master clock out
#define SAMPLE_VALID BIT7

#define FSPI_PORT_DIR P3DIR ///< direct port
#define FSPI_PORT_IN P3IN   ///< input port
#define FSPI_PORT_OUT P3OUT ///< output port
#define FSPI_PORT_SEL P3SEL ///< select port

#if defined MSP_16X
 #define FSPI_RX_BUF U0RXBUF
 #define FSPI_TX_BUF U0TXBUF
 #define FSPI_BUSY ((TXEPT & U0TCTL) == 0)
#elif defined MSP_261X
 #define FSPI_RX_BUF UCB0RXBUF
 #define FSPI_TX_BUF UCB0TXBUF
 #define FSPI_BUSY ((UCBUSY & UCB0STAT) != 0)
#else 
 #error "FSPI define"
#endif


/**
 \brief SPI initialization
 **/
void FSPI_Init(void);

/**
 \brief Disables SPI
 **/
void FSPI_Close(void);

/**
 \Brief Sends and receives 16bits over spi
 **/
inline unsigned int FSPI_write_wait_read(unsigned int datain);

#endif /* _FSPI_H_ */
