/*******************************************************************************
   display.c:
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

#include <fitkitlib.h>
#include "display.h"
#include "string.h"

int lcd_cursor_addr; ///< Pozice kurzoru na LCD (delka zapsaneho retezce)
int lcd_shift_addr;  ///< Offset zobrazeni na LCD

/**
 \brief Odeslání 8-bitového pøíkazu na LCD (data + RS bit).
 */

void LCD_send_cmd(unsigned char data, unsigned char rs) {
  unsigned char pkt[6];

  pkt[1] = data; pkt[0] = (rs & 0x1);       //EN=0
  pkt[3] = data; pkt[2] = 0x2 | (rs & 0x1); //EN=1
  pkt[5] = data; pkt[4] = (rs & 0x1);       //EN=0

  FPGA_SPI_RW_A8_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_DISPLAY, &(pkt[0]), 6);
}

/**
 \brief Vymazání LCD a nastavení kurzoru na první znak.
 */
void LCD_clear(void) {
  LCD_send_cmd(LCD_CLEAR_DISPLAY,0);   // vymazani displaya
  delay_ms(2);
  LCD_send_cmd(LCD_CURSOR_AT_HOME,0);  // vychozi pozice kurzoru
  delay_ms(2);

  lcd_cursor_addr = 0;
  lcd_shift_addr = 0;
}


/**
 \brief Inicializace LCD, vymazání a na nastavení režimu.
 */
void LCD_init(void){
  //vymazani displeje
  LCD_clear();

  // nastaveni modu - osmibitovy prenos, displej slozen ze dvou polovin, font 5x8
  LCD_send_cmd(LCD_FUNCTION_SET | LCD_LINES_TWO | LCD_DATA_LENGTH_8BIT | LCD_FONT_5x8,0);
  // zobrazeni kurzoru, zapnuti displeje
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON,0); 
  // automaticka inkrementace adresy kurzoru
  LCD_send_cmd(LCD_ENTRY_MODE_SET | LCD_CURSOR_MOVE_INC | LCD_SHIFT_DISABLE,0);
}


/**
 \brief Pøekódovací funkce, zatím jen opisuje vstup na vystup. 
  \n Možno využít v budoucnu na vykreslování znakù, definovaných v této funkci
     a uložených v CCRAM LCD.
 */
char LCD_char2dis(char c){
  return c;
}


/**
 \brief Nahraje znak do CGRAM (pamet znaku)
 \param address Adresa v CGRAM, kam se znak ma nahrat
 \param data Matice znaku (pole obsahujicich 8 bytu)
 */
void LCD_load_char(unsigned char address, unsigned char *data)
{
  unsigned char i;

  //adresa ma tvar 01XX XYYY, kde XXX je adresa znaku v CGRAM a YYY je adresa radku
  //nove definovany znak ma kod XXX (vlastni znaky jsou tedy v tabulce na pozicich 0x0 - 0x7)
  address = address << 3; 
  address |= 0x40;
  address &= 0xF8;
 
  for (i=0; i < LCD_FONT_5x8_BYTES; i++) {
    LCD_send_cmd(address | i, 0); //set CGRAM address
    delay_ms(1);
    LCD_send_cmd(data[i], 1);
  }

}

/**
 \brief Zápis znaku na LCD.
 */
void LCD_append_char(unsigned char ch) {
   //zapis do prvni poloviny
   LCD_send_cmd(LCD_SET_DDRAM_ADDR | lcd_cursor_addr, 0);
   LCD_send_cmd(LCD_char2dis(ch), 1);

   //zapis do druhe poloviny
   if (lcd_cursor_addr > (LCD_CHAR_COUNT/2 - 1)) {
      LCD_send_cmd(LCD_SET_DDRAM_ADDR |
                   (LCD_SECOND_HALF_OFS + lcd_cursor_addr - (LCD_CHAR_COUNT/2)), 0);
   } else {
      LCD_send_cmd(LCD_SET_DDRAM_ADDR |
                   (LCD_SECOND_HALF_OFS + lcd_cursor_addr - (LCD_CHAR_COUNT/2) + LCD_DDRAM_LENGTH), 0);
   }
   LCD_send_cmd(LCD_char2dis(ch), 1);

   //inkrement pocitadla
   lcd_cursor_addr++;
   if (lcd_cursor_addr > LCD_DDRAM_LENGTH)
      lcd_cursor_addr = LCD_DDRAM_LENGTH;
}  

/**
 \brief Zobrazení textového øetìzce na LCD
 */
void LCD_write_string(char *data) { 
   LCD_clear();
   LCD_append_string(data);
}

/**
 \brief Zápis textového øetìzce na LCD (na LCD se vejde max. 24 znakù)
 */
void LCD_append_string(char *data) { 
   int i, len;

   len = strlen(data);
   if (len + lcd_cursor_addr > LCD_DDRAM_LENGTH)
      len = LCD_DDRAM_LENGTH - lcd_cursor_addr;

   //zapis jednotlivych znaku
   for (i=0; i < len; i++)
      LCD_append_char(data[i]);
}

/**
 \brief Rotace textu na displeji o jeden znak vlevo
 \return Vrací 1, pokud je text ve výchozí pozici
 */
int LCD_rotate() {
  /*
  char i;
  if (lcd_shift_addr < lcd_cursor_addr ) {
      LCD_send_cmd(LCD_CURSOR_DISPLAY_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_DIR_LEFT,0); 
      lcd_shift_addr++;
  } else {
      //vypnout displej
      LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_OFF | LCD_CURSOR_OFF,0); 

      //posun kurzoru na zacatek
      LCD_send_cmd(LCD_CURSOR_AT_HOME,0);
      DelayMS(2);
      //rotace o displej doprava
      for (i=1; i < LCD_CHAR_COUNT; i++)
          LCD_send_cmd(LCD_CURSOR_DISPLAY_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_DIR_RIGHT,0); 

      //zapnout displej
      LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON,0); // zobrazeni kurzoru
      lcd_shift_addr = -LCD_CHAR_COUNT+1;
  }
  */
  LCD_send_cmd(LCD_CURSOR_DISPLAY_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_DIR_LEFT,0);
  return (lcd_shift_addr == 0) ? 1 : 0;
}
