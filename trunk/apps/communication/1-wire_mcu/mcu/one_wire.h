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


#ifndef _ONE_WIRE_H_
#define _ONE_WIRE_H_

#define ROM_BYTES   8     // device ROM bytes
#define MAX_DEVICES 5     // max. number of devices connected to the 1-wire bus

extern unsigned char found_ROMs[MAX_DEVICES][ROM_BYTES];  // table of found ROM codes
extern unsigned char num_ROMs;

#define ow_get_num_dev() num_ROMs

#define DQ_dir_out()     (P4DIR |= BIT3)
#define DQ_dir_in()      (P4DIR &= (~BIT3))
#define DQ_set_0()       (P4OUT &= (~BIT3))
#define DQ_set_1()       (P4OUT |= BIT3)
#define DQ_read()        ((P4IN & BIT3) == BIT3)

#define FALSE 0
#define TRUE 1


/*******************************************************************************
 * TimerB delay function, behaviour defined by timerb_init() - period 0.5425us
*******************************************************************************/
void delay_1us083(unsigned long us);


/*******************************************************************************
 * Delay macros used for 1-wire bus timing
*******************************************************************************/
#if defined MSP_16X
  #define delay_7us()   {unsigned char i = 3;   while(i) {asm("NOP"); i--;}}
  #define delay_60us()  {unsigned char i = 28;  while(i) {asm("NOP"); asm("NOP"); asm("NOP"); i--;}}
  #define delay_80us()  {unsigned char i = 37;  while(i) {asm("NOP"); asm("NOP"); asm("NOP"); i--;}}
  #define delay_420us() {unsigned char i = 205; while(i) {asm("NOP"); asm("NOP"); asm("NOP"); i--;}}
  #define delay_500us() {unsigned char i = 244; while(i) {asm("NOP"); asm("NOP"); asm("NOP"); i--;}}
#elif defined MSP_261X
  #define delay_7us()   delay_1us083(7)
  #define delay_60us()  delay_1us083(56) 
  #define delay_80us()  delay_1us083(74)
  #define delay_420us() delay_1us083(387)
  #define delay_500us() delay_1us083(460)
#else
  #error "Unknown CPU type"
#endif


/*******************************************************************************
 * 1-wire bus reset
*******************************************************************************/
unsigned char ow_reset(void);


/*******************************************************************************
 * read one bit
*******************************************************************************/
unsigned char ow_read_bit(void);


/*******************************************************************************
 * write one bit
*******************************************************************************/
void ow_write_bit(unsigned char bit);


/*******************************************************************************
 * read one byte
*******************************************************************************/
unsigned char ow_read_byte(void);


/*******************************************************************************
 * write one byte
*******************************************************************************/
void ow_write_byte(unsigned char byte);


/*******************************************************************************
 * 1-wire crc
*******************************************************************************/
unsigned char ow_crc(unsigned char x);


/*******************************************************************************
 * searches for devices connected to the 1-wire
*******************************************************************************/
void ow_find_devices(void);


/*******************************************************************************
 * perform match ROM
*******************************************************************************/
unsigned char ow_send_match_rom(unsigned char idx);


#endif /* _ONE_WIRE_H_ */
