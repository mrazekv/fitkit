/*******************************************************************************
   font_texture.c: Texture and font functions
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

#include <fitkitlib.h>
#include <stdio.h>
#include <string.h>
#include "font_texture.h"

T_item f_items[ITEMS];

// Vymazani videoram
// prebrano z projektu Textovy rezim
// http://merlin.fit.vutbr.cz/FITkit/docs/aplikace/apps_vga_textmode.html
void VGA_Clear(void)
{
  unsigned short i, j;
  unsigned char c;        
  for (i = 0; i < FONT_ROWS; i++) {
    for (j = 0; j < FONT_COLUMNS; j++) {
      // Odeslani dat do FPGA (2 byty adresy, 1 byte dat)
      c = 0;
      font_indexmap_write(j, i, &c);
    }
  }
}

// Vypsani textu
// prebrano z projektu Textovy rezim
// http://merlin.fit.vutbr.cz/FITkit/docs/aplikace/apps_vga_textmode.html
void VGA_TextOut(int pos_x, int pos_y, unsigned char *text) {
  unsigned char c;
  unsigned char *ch = text;

  while (*ch != '\0') {
    if (*ch > 31) {
      c = *ch - 32;
      font_indexmap_write(pos_x, pos_y, &c);
      pos_x++;
      if (pos_x > FONT_COLUMNS) {
        pos_x = 0;
        pos_y++;
      }
    }
    if (*ch == '\n') {
      pos_x = 0; 
      pos_y++;
    }
    if (pos_y > FONT_ROWS) break;
    ch++;
  }
}

//Nacteni fontu z flash, Inicializace blokove pameti FPGA
void Font_Flash_FPGA(void)
{
  unsigned char buf[16];
  int i, j;
  long addr;
  
  term_send_str("Loading font from flash ... ");
  addr = FLASH_PageToAddr(FONT_PAGE); //base

  for (i = 0; i < 128; i++) // 128 x 16B 
  {
    // Nacteni znaku z FLASH
    FLASH_ReadDataA((unsigned char *)&buf, FONT_HEIGHT, addr);
    
    //posun na dalsi znak
    addr += FONT_HEIGHT;
    
    // Odeslani do FPGA
    for (j = 0; j < 16; j++) {
        font_write(j,i, &(buf[j]));
    }
    WDG_reset();
  }
  term_send_str_crlf("OK");
}

// Oznaci polozku
void select_item(unsigned char sel)
{
  unsigned char c;
  unsigned char *ch = f_items[sel].s;
  unsigned char pos_x = f_items[sel].x;
  unsigned char pos_y = f_items[sel].y;
  
  while (*ch != '\0') {
    if (*ch > 31) {
      c = *ch - 32;
      c |= 128;
      font_indexmap_write(pos_x, pos_y, &c);
      pos_x++;
    }
    ch++;
  }
}
// Odznaci textovou polozku
void unselect_item(unsigned char sel)
{
  unsigned char c;
  unsigned char *ch = f_items[sel].s;
  unsigned char pos_x = f_items[sel].x;
  unsigned char pos_y = f_items[sel].y;
  
  
  while (*ch != '\0') {
    if (*ch > 31) {
      c = *ch - 32;
      font_indexmap_write(pos_x, pos_y, &c);
      pos_x++;
    }
    ch++;
  }
}

// Inicializace polozek
void items_init(void) {
  unsigned char text[20];
  sprintf(text,"Vybrat hru");
  f_items[0].x = 34;
  f_items[0].y = 9;
  strcpy(f_items[0].s, text);
  f_items[0].len = 10;
  
  sprintf(text,"Napoveda");
  f_items[1].x = 49;
  f_items[1].y = 14;
  strcpy(f_items[1].s, text);
  f_items[1].len = 8;
  
  sprintf(text,"O aplikaci");
  f_items[2].x = 34;
  f_items[2].y = 19;
  strcpy(f_items[2].s, text);
  f_items[2].len = 10;
  
  sprintf(text,"Statistiky");
  f_items[3].x = 19;
  f_items[3].y = 14;
  strcpy(f_items[3].s, text);
  f_items[3].len = 10;
  
  sprintf(text,"Zpet");
  f_items[4].x = 36;
  f_items[4].y = 25;
  strcpy(f_items[4].s, text);
  f_items[4].len = 6;
  
  sprintf(text,"Vymazat vysledky");
  f_items[5].x = 32;
  f_items[5].y = 24;
  strcpy(f_items[5].s, text);
  f_items[5].len = 16;
  
  sprintf(text,"LEVEL 1");
  f_items[6].x = 36;
  f_items[6].y = 9;
  strcpy(f_items[6].s, text);
  f_items[6].len = 7;
  
  sprintf(text,"LEVEL 2");
  f_items[7].x = 36;
  f_items[7].y = 10;
  strcpy(f_items[7].s, text);
  f_items[7].len = 7;
  
  sprintf(text,"LEVEL 3");
  f_items[8].x = 36;
  f_items[8].y = 11;
  strcpy(f_items[8].s, text);
  f_items[8].len = 7;
  
  sprintf(text,"LEVEL 4");
  f_items[9].x = 36;
  f_items[9].y = 12;
  strcpy(f_items[9].s, text);
  f_items[9].len = 7;
  
  sprintf(text,"LEVEL 5");
  f_items[10].x = 36;
  f_items[10].y = 13;
  strcpy(f_items[10].s, text);
  f_items[10].len = 7;
  
  sprintf(text,"LEVEL 6");
  f_items[11].x = 36;
  f_items[11].y = 14;
  strcpy(f_items[11].s, text);
  f_items[11].len = 7;
  
}

//Vypsani polozky
void print_item(unsigned char i) {
  
  unsigned char c;
  unsigned char *ch = f_items[i].s;
  int pos_x = f_items[i].x;
  int pos_y = f_items[i].y;
  
  while (*ch != '\0') {
    if (*ch > 31) {
      c = *ch - 32;
      font_indexmap_write(pos_x, pos_y, &c);
      pos_x++;
    }
    ch++;
  }
}

// inicializace hry, nahrani z flash do fpga
void init_board(unsigned char level)
{
  unsigned char buf[20];
  int i, j;
  long addr;

  term_send_str("Loading game level from FLASH ");
  // kazda hra zabira 303 bytu (300 pro pozice na desce + 3 informacni) 
  addr = FLASH_PageToAddr(LEVEL_PAGE) + level * 303 + 3; //base + offset
  
  for (i = 0; i < 15; i++)
  {
    // Nacteni bloku z FLASH
    FLASH_ReadDataA((unsigned char *)&buf, 20, addr);
    addr += 20;
    
    // Odeslani do FPGA
    for (j = 0; j < 20; j++)
    {
      texture_indexmap_write(j,i, &(buf[j]));
      }
    WDG_reset();
  }
  term_send_str_crlf("OK");
}

// vycisti pozici po hraci
void position_clear(unsigned char x, unsigned char y)
{
  unsigned char tex_id;
  texture_indexmap_read(x , y, &tex_id);
  // treti bit nemenime (v fpga slouzi k invertaci textury)
  tex_id = (tex_id & 4) + 1;
  texture_indexmap_write(x , y, &tex_id);
}

// umisteni textury na hraci plochu
void texture_set(unsigned char x, unsigned char y, unsigned char t)
{
  unsigned char tex_id;
  texture_indexmap_read(x, y, &tex_id);
  tex_id = (tex_id & 4)  + t;    
  texture_indexmap_write(x, y, &tex_id);
}
// aktualizace dat v FPGA
void Texture_Flash_FPGA(void)
{
  unsigned char buf[128];
  unsigned char data[2];
  int i, j, Clr;
  long addr;

  term_send_str("Loading texture from FLASH ");

  addr = FLASH_PageToAddr(TEXTURE_PAGE); //base
  for (i = 0; i < 32; i++) // 64x32 textura
  {
    // Nacteni bloku z FLASH
    FLASH_ReadDataA((unsigned char *)&buf, 128, addr);
    addr += 128;
    
    // Odeslani do FPGA
    for (j = 0; j < 64; j++)
    {
      Clr = buf[j * 2] + (buf[(j * 2) + 1] << 8);

      data[0] = (unsigned char)((Clr >> 8) & 255);
      data[1] = (unsigned char)(Clr & 255);
      texture_write(j,i, &(data[0]));
    }
    WDG_reset();
  }
  term_send_str_crlf("OK");
}
