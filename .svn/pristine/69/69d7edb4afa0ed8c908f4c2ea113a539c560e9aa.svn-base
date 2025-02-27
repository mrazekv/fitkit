/*******************************************************************************
   cmd.c:
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
     Rutiny pro command interpreter
*******************************************************************************/

/* Provedena revize */

#include "define.h"
#include "globfun.h"
#include "uart.h"
#include "cmd.h"

/**
 \brief Èteni znaku z terminálu.
 \return Znak naètený z RS232_0
 **/
#define term_read_after_break()     return term_rcv_char();

unsigned char CMDStr[MAX_COMMAND_LEN+1];  ///< Pole pøíchozích znakù.
volatile unsigned char CMDStrLen;                  ///< Poèet znakù v bufferu.
long          CMD_Long;                   ///< long èíslo pøeètené z RS232

/**
 \brief Vyprázdnìní bufferu znakù.
 **/
inline void term_cmd_clear(void) {
  CMDStrLen = 0;
}

/**
 \brief  Èteni pøikazu z terminálu.
 \return 0 - není konec ètení pøikazu,
         \n 1 - konec èteni pøikazu (pøišly znaky CRLF)
 **/
unsigned char term_cmd_read(void) {
  unsigned char ch;

  if ((ch = term_rcv_char()) != 0)  {
    switch (ch) {
      case BS:
        if (CMDStrLen != 0) {   // backspace
          CMDStrLen--;
          CMDStr[CMDStrLen] = 0;

          term_send_char(BS);   //smazani znaku v terminalu
          term_send_char(' ');
          term_send_char(BS);
        }
      break;

      case LF:                  // konec radku
        term_send_char(LF);
      break;

      case CR:                  // konec radku
        term_send_char(CR);
      break;

      default:                  // jine znaky
        if (CMDStrLen < MAX_COMMAND_LEN) {
          term_send_char(ch);
          CMDStr[CMDStrLen] = ch;
          CMDStrLen++;
          CMDStr[CMDStrLen] = 0;
        }
      break;
    } // switch
  }  // if

  if (ch != LF)
    return (0);
  
  return (1);
}

/**
  \brief  Dekodování pøíkazu.
  \return Èíselná hodnota dekódovaného pøíkazu
 **/
unsigned char term_cmd_decode(char *UString) {
        
  switch (CMDStrLen) {
    case 0:
      // neprisel zadny novy prikaz
      return (CMD_NULL);

    case 3:
      if (strcmp3("CLS", UString))
        return (CMD_CLEARSCREEN);
    break;

    case 4:
      if (strcmp4("HELP", UString))
        return (CMD_HELP);
    break;

    case 5:
      if (strcmp5("RAM D", UString))
        return (CMD_MCU_RAM_DUMP);
    break;

    case 9:
      if (strcmp9("PROG FPGA", UString))
        return (CMD_MCU_PROG_FPGAI);
      else if (strcmp9("RESET MCU", UString))
        return (CMD_MCU_RESET);
      else if (strcmp9("FLASH R S", UString))
        return (CMD_MCU_FLASHR_STATUS);
      else if (strcmp9("FLASH W X", UString))
        return (CMD_MCU_FLASHW_X);
      else if (strcmp9("SMCLK OFF", UString))
        return (CMD_SMCLK_STOP);
    break;

    case 10:
      if (strcmp10("RESET FPGA", UString))
        return (CMD_FPGA_RESET);
    break;

/*
    case 11:
      if (strcmp10("FLASH E ALL", UString))
        return (CMD_MCU_FLASHD_ALL);
    break;
*/
    case 12:
      if (strcmp10("FLASH W FPGA", UString))
        return (CMD_MCU_FLASHW_FPGA);
      else if (strcmp10("FLASH E FPGA", UString))
        return (CMD_MCU_FLASHD_FPGA);
    break;

    case 15:
      if (strcmp15("PROG FPGA FLASH", UString))
        return (CMD_MCU_FLASHP_FPGA);
    break;
  }

  if ((CMDStrLen > 9) && strcmp6("FLASH ", UString) &&
      strcmp1(" ", UString+7) && strcmp1(" ", UString+9)) {

    switch (UString[6]) {
      case 'R':
        if (UString[8] == 'A') {      // vypis z adresy flash
          CMD_Long = str2long((unsigned char *)&UString[10]);
          return (CMD_MCU_FLASHR_ADR);
        }
        else if (UString[8] == 'P') { // vypis stranky flash
          CMD_Long = str2long((unsigned char *)&UString[10]);
          return (CMD_MCU_FLASHR_PAGE);
        }
        else if (UString[8] == 'B') { // vypis bloku flash
          CMD_Long = str2long((unsigned char *)&UString[10]);
          return (CMD_MCU_FLASHR_BLOCK);
        }
      break;
/*
      case 'W':
        if (UString[8] == 'C') {      // zapis bytu do flash flash
          CMD_Char = UString[10];
          CMD_Long = str2long((unsigned char *)&UString[12]);
          return (CMD_MCU_FLASHW_C);
        }
      break;

      case 'E':
        if (UString[8] == 'A') {      // erase bytu flash
          CMD_Long = str2long((unsigned char *)&UString[10]);
          return (CMD_MCU_FLASHD_BYTE);
        }
        else if (UString[8] == 'P') { // erase stranky flash
          CMD_Long = str2long((unsigned char *)&UString[10]);
          return (CMD_MCU_FLASHD_PAGE);
        }
        else if (UString[8] == 'B') { // erase bloku flash
          CMD_Long = str2long((unsigned char *)&UString[10]);
          return (CMD_MCU_FLASHD_BLOCK);
        }
      break;
*/      
    }
  }

  // No way...
  return (CMD_UNKNOWN);
}

