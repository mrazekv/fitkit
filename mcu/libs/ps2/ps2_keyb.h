/*******************************************************************************
   ps2_keyb.h: PS2 KEYBOARD
   Copyright (C) 2008 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Pavel Bartos <xbarto41 AT stud.fit.vutbr.cz>

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
 
   This software is provided ``as is'', and any express or implied
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

#ifndef _PS2_KEYB_H_
#define _PS2_KEYB_H_


#include <spi/spi_config.h>

#ifndef BASE_ADDR_KEYBOARD
#define BASE_ADDR_KEYBOARD 0x0002 ///< Bazova adresa SPI pro PS/2 radic
#endif



/**
 \brief Inicializace klavesnice
 */
void keyboard_ps2_init(void (*callback)(unsigned char));

/**
 \brief Nastavi LED klavesnice
 */
void ps2_set_leds(unsigned char leds);

/**
 \brief Vrati stav LED klavesnice
 */
unsigned char ps2_get_leds();

/**
 \brief Automat klavesnice
 */
void keyboard_ps2_FM();

/**
 \brief Odesle byte do klavesnice
 */
void keyboard_ps2_send_char(unsigned char data);

#endif
