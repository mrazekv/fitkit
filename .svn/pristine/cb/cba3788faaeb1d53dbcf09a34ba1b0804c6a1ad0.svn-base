/*******************************************************************************
   main: rozhrani one wire
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

/*
 * Note:
 *
 * The source code is based o the source codes released in the following
 * application notes related to the DS18B20 digital thermometer device.
 *
 * AN126 - 1-Wire Communication Through Software
 * http://www.maxim-ic.com/app-notes/index.mvp/id/126
 * http://pdfserv.maxim-ic.com/en/an/AN126.pdf
 *
 * AN187 - 1-Wire Search Algorithm
 * http://www.maxim-ic.com/app-notes/index.mvp/id/187
 * http://pdfserv.maxim-ic.com/en/an/AN187.pdf
 * 
 */


#include <fitkitlib.h>

#include "one_wire.h"


/*******************************************************************************
 * 1-wire global wariables
*******************************************************************************/
unsigned char ROM[ROM_BYTES];                      // ROM bits
unsigned char last_discrep = 0;                    // last discrepancy
unsigned char done_flag = 0;                       // done flag
unsigned char found_ROMs[MAX_DEVICES][ROM_BYTES];  // table of found ROM codes
unsigned char num_ROMs;
unsigned char dowcrc;
unsigned char dscrc_table[] = {
    0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
  157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
   35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
  190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
   70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
  219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
  101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
  248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
  140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
   17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
  175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
   50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
  202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
   87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
  233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
  116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};


/*******************************************************************************
 * TimerB delay function, behaviour defined by timerb_init() - period 0.5425us
*******************************************************************************/
void delay_1us083(unsigned long us)
{
#if defined MSP_16X
#elif defined MSP_261X
  TBCCR1 = (us << 1) + TBR;
  while (TBR != TBCCR1) {}
#else
  #error "Unknown CPU type"
#endif

}


/*******************************************************************************
 * 1-wire bus reset
*******************************************************************************/
unsigned char ow_reset(void)
{
	unsigned long presence;    // 0 - presence, 1 - no part

	DQ_dir_out();              // set output
	DQ_set_0();                // and set it to 0
	delay_500us();
	DQ_dir_in(); 	             // set input
	delay_60us();

  presence = DQ_read();

	delay_420us();

  return presence;
}


/*******************************************************************************
 * read one bit
*******************************************************************************/
unsigned char ow_read_bit(void)
{
  unsigned char bit;

  delay_7us();
  
  DQ_dir_out();           // out
  DQ_set_0();             // 0
  delay_7us();
  DQ_dir_in();            // in
  delay_7us();

  bit = DQ_read();        // read bus

  delay_80us();

  return(bit);
}


/*******************************************************************************
 * write one bit
*******************************************************************************/
void ow_write_bit(unsigned char bit)
{
  DQ_dir_out();     // out
  DQ_set_0();       // 0
  delay_7us();

  if (bit)
    DQ_set_1();     // 1
  else
    DQ_set_0();     // 0
  delay_80us();

  DQ_dir_in();
  delay_7us();
}


/*******************************************************************************
 * read one byte
*******************************************************************************/
unsigned char ow_read_byte(void)
{
  unsigned char i;
  unsigned char byte = 0;

  for (i = 0; i < 8; i++)
  {
    if (ow_read_bit())
      byte |= 0x01 << i;
  }

  return byte;
}


/*******************************************************************************
 * write one byte
*******************************************************************************/
void ow_write_byte(unsigned char byte)
{
  unsigned char i;
  
  for (i = 0; i < 8; i++)
  {
    ow_write_bit((byte >> i) & 0x01);
  }
}


/*******************************************************************************
 * 1-wire crc
*******************************************************************************/
unsigned char ow_crc(unsigned char x)
{
  dowcrc = dscrc_table[dowcrc^x];
  return dowcrc;
}


/*******************************************************************************
 * next
 * The function searches for the next device on the 1-wire bus. If
 * there are no more devices on the 1-wire then false is returned.
*******************************************************************************/
unsigned char next(void)
{
  unsigned char bit_i = 1;          // ROM bit index
  unsigned char byte_i = 0;         // ROM byte index
  unsigned char bit_mask = 1;       // bit mask
  unsigned char x = 0;
  unsigned char discrep_marker = 0; // discrepancy marker
  unsigned char out_bit;            // output bit
  unsigned char nxt;                // return value
  int flag;

  nxt = FALSE;                      // set the next flag to false
  dowcrc = 0;                       // reset the dowcrc
  flag = ow_reset();                // reset the 1-wire

  if (flag || done_flag)            // no parts -> return false
  {
    last_discrep = 0;               // reset the search
    return FALSE;
  }

  ow_write_byte(0xF0);                       // send search ROM command
  do                                         // for all eight bytes
  {
    x = 0;
    if (ow_read_bit())
      x = 0x02;
    if (ow_read_bit())
      x |= 0x01;                             // and its complement
    if (x == 0x03)                           // there are no devices on the 1-wire
      break;
    else
    {
      if(x > 0)                              // all devices coupled have 0 or 1
        out_bit = x >> 1;                    // bit write value for search
      else
      {
                                             // if this discrepancy is before the last
                                             // discrepancy on a previous next call then pick
                                             // the same as last time
        if (bit_i < last_discrep)
          out_bit = ((ROM[byte_i] & bit_mask) > 0);
        else                                 // if equal to last pick 1
          out_bit = (bit_i == last_discrep); // if not then pick 0
                                             // if 0 was picked then record
                                             // position with mask bit_mask
        if (out_bit == 0)
          discrep_marker = bit_i;
      }
      if (out_bit == 1)                      // isolate bit in ROM[byte_i] with mask bit_mask
        ROM[byte_i] |= bit_mask;
      else
        ROM[byte_i] &= ~bit_mask;
      ow_write_bit(out_bit);                 // ROM search write
      bit_i++;                               // increment bit counter bit_i
      bit_mask = bit_mask << 1;              // and shift the bit mask bit_mask
      if (bit_mask == 0)                     // if the mask is 0 then go to new ROM
      {                                      // byte byte_i and reset mask
        ow_crc(ROM[byte_i]);                 // accumulate the CRC
        byte_i++; bit_mask++;
      }
    }
  } while (byte_i < ROM_BYTES);              // loop until through all ROM bytes 0-7

  if (bit_i < 65 || dowcrc)                  // if search was unsuccessful then
    last_discrep = 0;                        // reset the last discrepancy to 0
  else
  {
                                             // search was successful, so set last_discrep,
                                             // done_flag, nxt
    last_discrep = discrep_marker;
    done_flag = (last_discrep == 0);
    nxt = TRUE;                              // indicates search is not complete yet, more
                                             // parts remain
  }

  return nxt;

}


/*******************************************************************************
 * first
 * The function resets the current state of a ROM search and calls
 * function next to find the first device on the 1-wire bus.
*******************************************************************************/
unsigned char first(void)
{
  last_discrep = 0;                          // reset the ROM search last discrepancy global
  done_flag = FALSE;
  return next();                             // call next and return its return value
}


/*******************************************************************************
 * searches for devices connected to the 1-wire
*******************************************************************************/
void ow_find_devices(void)
{
  unsigned char m;

  if (!ow_reset())                                  // begins when a presence is detected
  {
    if (first())                                    // begins when at least one part is found
    {
      num_ROMs = 0;
      do
      {
        //num_ROMs++;
        for (m = 0; m < ROM_BYTES; m++)
        {
          found_ROMs[num_ROMs][m] = ROM[m];         // identifies ROM
                                                    // number on found device
        }
        num_ROMs++;
      } while (next() && (num_ROMs < MAX_DEVICES)); // continues until no additional devices are found
    }
  }

}


/*******************************************************************************
 * perform match ROM
*******************************************************************************/
unsigned char ow_send_match_rom(unsigned char idx)
{
  unsigned char i;
  if (ow_reset())
    return FALSE;
  ow_write_byte(0x55);                      // match ROM
  for (i = 0; i < ROM_BYTES; i++)
  {
    ow_write_byte(found_ROMs[idx][i]);      // send ROM code
  }
  return TRUE;

}
