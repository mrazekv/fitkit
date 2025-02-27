/*******************************************************************************
   cmd.h:
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

#ifndef _CMD_H_
#define _CMD_H_

#include "define.h"

extern unsigned char CMDStr[MAX_COMMAND_LEN+1];  ///< Pole pøíchozích znakù.
extern volatile unsigned char CMDStrLen;                  ///< Poèet znakù v bufferu.
extern long CMD_Long;                   ///< long èíslo pøeètené z RS232


/**
 \brief Vyprázdnìní bufferu znakù.
 **/
inline void term_cmd_clear(void);
//#define CMD_Clear() term_cmd_clear() 

/**
 \brief  Èteni pøikazu z terminálu.
 \return 0 - není konec ètení pøikazu,
         \n 1 - konec èteni pøikazu (pøišly znaky CRLF)
 **/
unsigned char term_cmd_read(void);
//#define CMD_Read() term_cmd_read() 

/**
  \brief  Dekodování pøíkazu.
  \return Èíselná hodnota dekódovaného pøíkazu
 **/
unsigned char term_cmd_decode(char *UString);
//#define CMD_Decode() term_cmd_decode()

/**
  \brief  Dekódování pøíkazù definovaných uživatelem (programátorem aplikace).
  \param  UString - uppercase pøíchozí øetìzec (pro porovnaní pøíkazu)
  \param  String - puvodní pøíchozí øetezec (pro další využití)
  \return CMD_UNKNOWN - neznamý pøíkaz
          \n USER_COMMAND - pøíkaz korektnì dekódován
 **/
unsigned char term_usr_cmd_decode(char *UString, char *String);
//#define USER_CMD_Decode(usrt, str) term_usr_cmd_decode(usrt, str)

#endif   /* _CMD_H_ */
