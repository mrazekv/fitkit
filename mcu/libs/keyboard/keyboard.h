/*******************************************************************************
   keyboard.h:
   Copyright (C) 2006 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jan Markovic <xmarko04@stud.fit.vutbr.cz>

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

   popis:


   $Id$

*******************************************************************************/

#ifndef _KEYBOARD_4x4_H_
#define _KEYBOARD_4x4_H_

#include <spi/spi_config.h>

#ifndef BASE_ADDR_KEYBOARD
   #define BASE_ADDR_KEYBOARD 0x02           ///< Bázová adresa SPI pro øadiè keyboard_controller
#endif

#ifndef KEYBOARD_BUFFER_LENGTH
   #define KEYBOARD_BUFFER_LENGTH 20     ///< Délka vyrovnávacího Rx bufferu pro FPGA
#endif

#define KEY_0 0x0080 ///< definice bitu, ktery reprezentuje klavesu '0' na klavesnici
#define KEY_1 0x0001 ///< definice bitu pro klavesu '1' na klavesnici
#define KEY_2 0x0010 ///< definice bitu pro klavesu '2' na klavesnici
#define KEY_3 0x0100 ///< definice bitu pro klavesu '3' na klavesnici
#define KEY_4 0x0002 ///< definice bitu pro klavesu '4' na klavesnici
#define KEY_5 0x0020 ///< definice bitu pro klavesu '5' na klavesnici
#define KEY_6 0x0200 ///< definice bitu pro klavesu '6' na klavesnici
#define KEY_7 0x0004 ///< definice bitu pro klavesu '7' na klavesnici
#define KEY_8 0x0040 ///< definice bitu pro klavesu '8' na klavesnici
#define KEY_9 0x0400 ///< definice bitu pro klavesu '9' na klavesnici
#define KEY_h 0x0008 ///< definice bitu pro klavesu '*' na klavesnici
#define KEY_m 0x0800 ///< definice bitu pro klavesu '#' na klavesnici
#define KEY_A 0x1000 ///< definice bitu pro klavesu 'A' na klavesnici
#define KEY_B 0x2000 ///< definice bitu pro klavesu 'B' na klavesnici
#define KEY_C 0x4000 ///< definice bitu pro klavesu 'C' na klavesnici
#define KEY_D 0x8000 ///< definice bitu pro klavesu 'D' na klavesnici


/**
 \brief Ètení aktuálního stavu klávesnice.
 \return Aktualní hodnota stavu klávesnice
 */
#define read_word_keyboard_4x4() \
        FPGA_SPI_RW(A8_D16, SPI_FPGA_ENABLE_READ, BASE_ADDR_KEYBOARD, 0)

/**
 \brief Pøekódování 16 bitových dat z klávesnice na char.
 \param in_ch - 16-bitovy stav z klávesnice
 \return Znaková reprezentace zmáènuté klávesy
 */
char key_decode(unsigned int in_ch);

/**
 \brief Inicializace pro obsluhu klávesnice 4x4 v MCU.
 */
//void keyboard_init();
extern volatile unsigned char index_buf;
extern volatile unsigned char write_index_buf;
extern volatile unsigned char chars_in_buf;

#define keyboard_init() { index_buf = write_index_buf = chars_in_buf = 0; }

/**
 \brief Poèet znakù ve vstupním bufferu klavesnice
 \return poèet znakù
 */
#define keyboard_in_buf() chars_in_buf


/**
 \brief Ètení znaku z bufferu klavesnice 4x4.
 \return Znak (8 bitù)
 **/
unsigned char keyboard_get_char(void);


/**
 \brief Obsluha pøerušení - pøíjem informací o prerušení z FPGA pøes SPI a uložení informací do bufferu.
 */
void keyboard_handle_interrupt(void);

#endif  //_KEYBOARD_4x4_H_
