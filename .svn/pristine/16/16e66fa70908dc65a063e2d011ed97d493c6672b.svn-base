/*******************************************************************************
   texture_font.h: Texture and font function include file
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jiri Subr <xsubrj00 AT stud.fit.vutbr.cz>

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
#ifndef _FONT_TEXTURE_H_
#define _FONT_TEXTURE_H_

#define ITEMS                12       // pocet textovych polozek -- nevim asi smaznout presunout jinam

//parametry textury a obrazovky s texturami
#define TEXTURE_WIDTH   64              // sirka cele textury
#define TEXTURE_COLUMNS 20              // pocet textur na radku
#define TEXTURE_ROWS    15              // pocet textur v sloupci

#define BOARD_MAXCOLUMNS  128           // maximalni sirka hraci desky

//parametry fontu a obrazovky s fonty
#define FONT_HEIGHT          16         // vyska fontu
#define FONT_COLUMNS         80         // pocet znaku na radku
#define FONT_ROWS            30         // pocet radku znaku
/*******************************************************************************
 * Adresy ve Flash pameti
*******************************************************************************/
#define TEXTURE_PAGE          FLASH_USER_DATA_PAGE // Adresa textury ve FLASH
#define TEXTURE_SIZE_PAGE     16                   // Velikost textury (stranky)
//adresa fontu ve FLASH
#define FONT_PAGE             FLASH_USER_DATA_PAGE + TEXTURE_SIZE_PAGE
#define FONT_SIZE_PAGE        8                    // Velikost fontu (stranky)
// Adresa urovni(levelu) ve FLASH
#define LEVEL_PAGE            FONT_PAGE + FONT_SIZE_PAGE
#define LEVELS_SIZE_PAGE      7                    // Velikost urovni (stranky)
// Adresa statistik ve FLASH
#define STATISTICS_PAGE       LEVEL_PAGE + LEVELS_SIZE_PAGE
// Velikost statistik (stranky)
#define STATISTICS_SIZE_PAGE  1
/*******************************************************************************
 * Adresy blokovych pameti
*******************************************************************************/ 
#define BASE_ADDR_TEXTURE    0x1000              //bazova adresa pameti textur
#define BASE_ADDR_IDXMAP     0x2000              //bazova adresa video pameti
#define BASE_ADDR_FONT       0x8000              //bazova adresa pameti fontu
/*******************************************************************************
 * Makra pro komunikaci pres SPI
*******************************************************************************/
// TEXTURA
// prenos textury do blokove pameti
#define texture_write(x, y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, \
        BASE_ADDR_TEXTURE | (((y) * TEXTURE_WIDTH) + (x)), data, 2, 2)
// zapis dat do blokove pameti indexu
#define texture_indexmap_write(x,y, data) \
        font_indexmap_write((x) * 4,(y) * 2, data)
#define texture_indexmap_read(x,y, data) \
        font_indexmap_read((x) * 4,(y) * 2, data)
        
// FONT
// prenos fontu do blokove pameti
#define font_write(x, y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, \
        BASE_ADDR_FONT | (((y) * FONT_HEIGHT) + (x)), data, 2, 1);
// zapis dat do blokove pameti indexu
#define font_indexmap_write(x,y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_IDXMAP | (((y) *\
        BOARD_MAXCOLUMNS) + (x)), data, 2, 1)
// cteni dat z blokove pameti indexu
#define font_indexmap_read(x,y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, BASE_ADDR_IDXMAP | (((y) *\
        BOARD_MAXCOLUMNS) + (x)), data, 2, 1)

#define on_board(x,y) \
        (((x) < TEXTURE_COLUMNS) && ((y) < TEXTURE_ROWS))

/*******************************************************************************
 * STRUKTURY
*******************************************************************************/
// struktura pro smer pohybu po hraci plose
typedef struct move {
  signed char x;
  signed char y;
}T_move;

// struktura pro ulozeni polozky menu
typedef struct item {
  unsigned char x;
  unsigned char y;
  unsigned char s[20];
  unsigned char len;
}T_item;

/*******************************************************************************
 * Prototypy funkci
*******************************************************************************/

// nacteni textury z flash do fpga
void Texture_Flash_FPGA(void);
// inicializace hry
void init_board(unsigned char level);
// umisteni textury na hraci plochu
void texture_set(unsigned char x, unsigned char y, unsigned char t);
// vycisteni pozice
void position_clear(unsigned char x, unsigned char y);
// vycisteni obrazovky (pameti indexu)        
void VGA_Clear(void);
// vypise text na obrazovku
void VGA_TextOut(int pos_x, int pos_y, unsigned char *text);
// nacteni fontu z flash do fpga
void Font_Flash_FPGA(void);
// inicializace polozek
void items_init(void);
// zobrazeni polozky
void print_item(unsigned char i);
// oznaceni vybrazen polozky
void select_item(unsigned char sel);
// odznaceni vybrazen polozky
void unselect_item(unsigned char sel);

#endif  // _FONT_TEXTURE_H_
