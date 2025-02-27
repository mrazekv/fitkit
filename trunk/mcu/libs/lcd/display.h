/*******************************************************************************
   display.h:
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

    Poznamka:
          Nejmene vyznamovy bit adresy odpovida bitu RS (viz VHDL kod a zapojeni
      radice LCD k SPI dekoderu).

   $Id$

*******************************************************************************/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <spi/spi_config.h>

#ifndef BASE_ADDR_DISPLAY
   #define BASE_ADDR_DISPLAY 0x00 ///< Bázová adresa SPI pro øadiè lcd_controller
#endif


/**
 \brief Instrukce pro LCD radic
 */
#define LCD_CLEAR_DISPLAY     0x01
#define LCD_CURSOR_AT_HOME    0x02
#define LCD_ENTRY_MODE_SET    0x04
#define LCD_DISPLAY_ON_OFF    0x08
#define LCD_CURSOR_DISPLAY_SHIFT  0x10
#define LCD_FUNCTION_SET      0x20
#define LCD_SET_CGRAM_ADDR    0x40
#define LCD_SET_DDRAM_ADDR    0x80

//LCD_ENTRY_MODE_SET
#define LCD_CURSOR_MOVE_DEC   0x00
#define LCD_CURSOR_MOVE_INC   0x02
#define LCD_SHIFT_ENABLE      0x01
#define LCD_SHIFT_DISABLE     0x00

//LCD_DISPLAY_ON_OFF
#define LCD_DISPLAY_ON        0x04
#define LCD_DISPLAY_OFF       0x00
#define LCD_CURSOR_ON         0x02
#define LCD_CURSOR_OFF        0x00
#define LCD_BLINKING_ON       0x01
#define LCD_BLINKING_OFF      0x00

//LCD_CURSOR_DISPLAY_SHIFT
#define LCD_SHIFT_DISPLAY     0x08
#define LCD_SHIFT_CURSOR      0x00
#define LCD_SHIFT_DIR_RIGHT   0x04
#define LCD_SHIFT_DIR_LEFT    0x00

//LCD_FUNCTION_SET
#define LCD_DATA_LENGTH_8BIT  0x10
#define LCD_DATA_LENGTH_4BIT  0x00
#define LCD_LINES_TWO         0x08
#define LCD_LINES_ONE         0x00
#define LCD_FONT_5x11         0x04
#define LCD_FONT_5x8          0x00
#define LCD_FONT_5x8_BYTES    0x08 // pocet bytu definice znaku 5x8


//Definice parametru displeje
#if defined LCD_ROWS_1
  #define LCD_CHAR_COUNT        0x10 // pocet znaku displeje FITKit 1
#elif defined LCD_ROWS_2
  #define LCD_CHAR_COUNT        0x20 // pocet znaku displeje FITKit 2
#endif

#define LCD_FIRST_HALF_OFS    0x00 // offset prvni pulky
#define LCD_SECOND_HALF_OFS   0x40 // offset druhe pulky
#define LCD_DDRAM_LENGTH      0x28 // pocet bytu jedne pulky
/**
 \brief Odeslání 8-bitových dat na LCD (data + RS).
 */
void LCD_send_cmd(unsigned char data, unsigned char rs);


/**
 \brief Vymazání LCD a nastavení kurzoru na první znak.
 */
void  LCD_clear(void);


/**
 \brief Inicializace LCD, vymazání a na nastavení režimu.
 */
void LCD_init(void);


/**
 \brief Pøekódovací funkce, zatím jen opisuje vstup na vystup. 
  \n Možno využít v budoucnu na vykreslování znakù, definovaných v této funkci
     a uložených v CCRAM LCD.
 */
char LCD_char2dis(char c);


/**
 \brief Zápis znaku na LCD.
 */
void LCD_append_char(unsigned char ch);

/**
 \brief Zobrazení textového øetìzce na LCD (pøed zápisem dojde k vymazání LCD)
 */
void LCD_write_string(char *data);

/**
 \brief Výpis textového øetìzce na LCD
 */
void LCD_append_string(char *data);

/**
 \brief Rotace textu na displeji o jeden znak vlevo
 \return Vrací 1, pokud je text ve výchozí pozici
 */
int LCD_rotate();


/**
 \brief Nahraje znak do CGRAM (pamet znaku)
 \param address Adresa v CGRAM, kam se znak ma nahrat
 \param data Matice znaku (pole obsahujicich 8 bytu)
 */
void LCD_load_char(unsigned char address, unsigned char *data);

#endif  // _DISPLAY_H_

