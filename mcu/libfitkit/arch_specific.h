/*******************************************************************************
   arch_specific.h: Specificke definice pro architekturu a prekladac
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
*******************************************************************************/

#ifndef _ARCH_SPECIFIC_H_
#define _ARCH_SPECIFIC_H_

#if FITKIT_VERSION == 1
  #include <msp430x16x.h> 
  #define MSP_16X
  #define LCD_ROWS_1
#elif FITKIT_VERSION == 2
  #include <msp430x261x.h> 
  #define MSP_261X
  #define LCD_ROWS_2
#else
    #error "Unknown FITKIT_VERSION"
#endif

/* nastaveni komunikacni rychlosti */
/*                               Fitkit 1.x, Fitkit 2.x
 * SMCLK = 7.3728 MHz
 * 
 * U0BR0 = 0x08; U0BR1 = 0x00; // SMCLK / 8    = 921600
 * U0BR0 = 0x10; U0BR1 = 0x00; // SMCLK / 16   = 460800 
 * U0BR0 = 0x20; U0BR1 = 0x00; // SMCLK / 32   = 230400 
 * U0BR0 = 0x40; U0BR1 = 0x00; // SMCLK / 64   = 115200
 * U0BR0 = 0x80; U0BR1 = 0x00; // SMCLK / 128  = 57600
 * U0BR0 = 0xC0; U0BR1 = 0x00; // SMCLK / 192  = 38400
 * U0BR0 = 0x80; U0BR1 = 0x01; // SMCLK / 384  = 19200
 * U0BR0 = 0x00; U0BR1 = 0x03; // SMCLK / 768  = 9600
 * U0BR0 = 0x00; U0BR1 = 0x06; // SMCLK / 1536 = 4800
 * U0BR0 = 0x00; U0BR1 = 0x0C; // SMCLK / 3072 = 2400
 * U0BR0 = 0x00; U0BR1 = 0x18; // SMCLK / 6144 = 1200
 */

//#if defined MSP_16X
  #define UART_BR0 0x10
  #define UART_BR1 0x00
/*#elif defined MSP_261X
  #define UART_BR0 0x20 
  #define UART_BR1 0x00
#else
  #error "Unknown CPU type"
#endif
*/

#include <signal.h>
#include <isr_compat.h>

#endif /* _ARCH_SPECIFIC_H_ */
