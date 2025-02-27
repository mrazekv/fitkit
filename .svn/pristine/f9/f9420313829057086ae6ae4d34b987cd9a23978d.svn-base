/*******************************************************************************
   display_cz.h: Knihovna pro obsluhu LCD displeje, vƒçetnƒõ ƒçesk√Ωch znak≈Ø.
   Copyright (C) 2008 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Pavel Barto≈° <xbarto41 AT stud.fit.vutbr.cz>

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

*******************************************************************************/
#ifndef _DISPLAY_CZ_H
#define _DISPLAY_CZ_H

#include <spi/spi_config.h>

#ifndef BASE_ADDR_DISPLAY
   #define BASE_ADDR_DISPLAY 0x00 ///< B·zov· adresa SPI pro ¯adiË lcd_controller
#endif

typedef const char Tcharmatrix[8];
typedef const char *Tcodetable[128];

/**
 \brief Nahraje ÔøΩeskÔøΩ/speciÔøΩlnÔøΩ znak do cgram a vrÔøΩtÔøΩ jeho adresu
 */
unsigned char lcd_cz_char(unsigned char ch);

/**
 \brief ZneplatnÔøΩ cgram 
 */
void lcd_cz_clear_cgram();

/**
 \brief Nahraje znak do cgram
 \param address Adresa v cgram, kam se znak mÔøΩ nahrÔøΩt
 \param data Matice znaku
 */
void lcd_cz_load_char(unsigned char address, const char *data);



/**
 \brief Inicializace LCD
 */
void lcd_init();

/**
 \brief VymazÔøΩnÔøΩ dipleje
 */
void lcd_clear();

/**
 \brief PÔøΩÔøΩmÔøΩ zÔøΩpis do LCD ÔøΩadiÔøΩe
 \param data Data
 \param RS ÔøΩroveÔøΩ RS vodiÔøΩe
 */
void lcd_raw(unsigned char data, unsigned char RS);

/**
 \brief VrÔøΩtÔøΩ aktuÔøΩlnÔøΩ pozici kurzoru
 */
char lcd_get_pos();

/**
 \brief NastavÔøΩ pozici kurzoru
 */
void lcd_set_pos(char pos);

/**
 \brief OdeÔøΩle do LCD displeje ÔøΩetÔøΩzec
 */
void lcd_string(const char *string);

/**
 \brief OdeÔøΩle znak na LCD
 */
void lcd_char (unsigned char ch);

/**
 \brief Nastavi kodovaci tabulku
 */
void lcd_set_code_table(Tcodetable *code_table);

#endif //_DISPLAY_CZ_H
