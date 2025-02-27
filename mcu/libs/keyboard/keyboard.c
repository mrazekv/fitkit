/*******************************************************************************
   keyboard.c:
   Copyright (C) 2006 Brno University of Technology,
                      Faculty of Information Technology
   Copyright (C) 2006 CAMEA ltd. & R.B.E
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


#include <fitkitlib.h>
#include "keyboard.h"


volatile unsigned char keyboard_buffer[KEYBOARD_BUFFER_LENGTH]; ///< buffer pro pøíjem dat z klavesnice (IRQ)
volatile unsigned char index_buf; ///< ètecí index v bufferu
volatile unsigned char write_index_buf; ///< zápisový index v bufferu
volatile unsigned char chars_in_buf; ///< poèet bytù (znakù) v bufferu

/**
 \brief Ètení znaku z bufferu klavesnice 4x4.
 \return Znak (8 bitù)
 **/
unsigned char keyboard_get_char(void)
{
  unsigned char Uchar;
  Uchar = 0;
  if (chars_in_buf > 0)
  {
    Uchar = keyboard_buffer[index_buf];
    index_buf++;
    if (index_buf >= KEYBOARD_BUFFER_LENGTH)
      index_buf = 0;
    chars_in_buf--;
  }
  return (Uchar);
}


/**
 \brief Obsluha pøerušení - pøíjem informací o prerušení z FPGA pøes SPI a uložení informací do bufferu.
 */
void keyboard_handle_interrupt(void)
{

  unsigned char ch = key_decode(read_word_keyboard_4x4());

  if (chars_in_buf < KEYBOARD_BUFFER_LENGTH)
  {
    keyboard_buffer[write_index_buf] = ch;  // zapis dat do buffru
    write_index_buf++;
    if (write_index_buf >= KEYBOARD_BUFFER_LENGTH)
      write_index_buf = 0;
    chars_in_buf++;
  }
}



/**
 \brief Pøekódování 16 bitových dat z klávesnice na char.
 \param in_ch - 16-bitovy stav z klávesnice
 \return Znaková reprezentace zmáènuté klávesy
 */
char key_decode(unsigned int in_ch)
{

  if (in_ch & KEY_0)
  {
  //    RS232_SendChar('0');
      return '0';
  }

  if (in_ch & KEY_1){
  //    RS232_SendChar('1');
      return '1';
  }


 if (in_ch & KEY_2){
  //    RS232_SendChar('2');
      return '2';
  }

 if (in_ch & KEY_3){
  //    RS232_SendChar('3');
      return '3';
  }

 if (in_ch & KEY_4){
  //    RS232_SendChar('5');
      return '4';
  }

 if (in_ch & KEY_5){
      return '5';
  }

 if (in_ch & KEY_6){
      return '6';
  }

 if (in_ch & KEY_7){
      return '7';
  }

 if (in_ch & KEY_8){
      return '8';
  }

 if (in_ch & KEY_9){
      return '9';
  }

 if (in_ch & KEY_h){
      return '*';
  }

 if (in_ch & KEY_m){
      return '#';
  }

 if (in_ch & KEY_A){
      return 'A';
  }

 if (in_ch & KEY_B){
      return 'B';
  }

 if (in_ch & KEY_C){
      return 'C';
  }

 if (in_ch & KEY_D){
      return 'D';
  }
  return 0;
}

