/*******************************************************************************
   audio_codec.c: FITKit 2.0 codec control functions
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

#include <fitkitlib.h>
#include "audio_codec.h"

/*******************************************************************************
 * function sends codec configuration data via SPI
 ******************************************************************************/
void codec_SPI_write(unsigned int addr, unsigned int data)
{
  char dataH, dataL;
  dataH = (addr << 1) | ((data >> 8) & 1);
  dataL = data & 0xFF;

  /*
  term_send_str("Sending to Codec: ");
  term_send_hex((addr << 9) | data);
  term_send_str(" H: ");
  term_send_hex(dataH);
  term_send_str(" L: ");
  term_send_hex(dataL);
  term_send_str(" addr: ");
  term_send_hex(addr);
  term_send_str(" reg: ");
  term_send_hex(data);
  term_send_crlf();
  */
  

  /* the ACLK pin acts as negCS */
  P5SEL &= ~ACLK;  /* disable ACLK on this pin */
  P5OUT &= ~ACLK;  /* set negCS signal to 0 */

  /* send high and low byte of the configuration word */
  SPI_TX_BUF = dataH;
  while (SPI_BUSY) WDG_reset();
  SPI_TX_BUF = dataL;
  while (SPI_BUSY) WDG_reset();

  P5OUT |= ACLK;  /* set negCS signal to 1 */
  /*
  delay_ms(1);
  */
}
