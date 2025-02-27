/*******************************************************************************
   vga_text.c: Text mode
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>

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

#include "vga_text.h"

//sloupcu
#define VGA_COLUMNS 80
//radku
#define VGA_ROWS    30
//pocet znaku na radek
#define VGA_STRIDE  128

unsigned short vga_pos = 0;

//Vymazani videoram
void VGA_Clear(void)
{
  unsigned short i, j;
  unsigned char c;        
  vga_pos = 0;
  for (i = 0; i < VGA_ROWS; i++) {
    for (j = 0; j < VGA_COLUMNS; j++) {
      // Odeslani dat do FPGA (2 byty adresy, 1 byte dat)
      c = 0;
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0xA000 /*base*/ + i*VGA_STRIDE + j, &c, 2, 1);
    }
  }
}

//Zmena pozice
void VGA_GotoXY(unsigned char x, unsigned char y) {
  if ((x > VGA_COLUMNS) || (y > VGA_ROWS)) return;

  vga_pos = (unsigned short)y * VGA_STRIDE + (unsigned short)x;
}

//Vypsani textu
void VGA_TextOut(char *text) {
  unsigned char c;
  unsigned char *ch = text;
  unsigned char vga_pos_x = vga_pos % VGA_STRIDE;
  unsigned char vga_pos_y = vga_pos / VGA_STRIDE;

  while (*ch != '\0') {  
    if (*ch > 31) {
      c = *ch - 32;
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0xA000 /*base*/ + vga_pos, &c, 2, 1);

      vga_pos_x++;
      if (vga_pos_x > VGA_COLUMNS) { 
        vga_pos_x = 0;
        vga_pos_y++;
      }
      vga_pos = vga_pos_y*VGA_STRIDE + vga_pos_x;
    }
    if (*ch == '\n') {
      vga_pos_x = 0; 
      vga_pos_y++;
      vga_pos = vga_pos_y*VGA_STRIDE + vga_pos_x;
    }
    if (vga_pos_y > VGA_ROWS) break;
    ch++;
  }          
}

//Rolovani o jeden radek nahoru
void VGA_ScrollUp(void) {
  unsigned short i,j;        
  unsigned short addr;
  unsigned char val;

  if (vga_pos > VGA_STRIDE) {
    vga_pos -= VGA_STRIDE;
  } else {
    vga_pos = 0;           
  }

  for (i = 1; i < VGA_ROWS; i++) {
    for (j = 0; j < VGA_COLUMNS; j++) {
      addr = 0xA000 /*base*/ + i * VGA_STRIDE + j;
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, addr, &val, 2, 1);
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, addr-VGA_STRIDE, &val, 2, 1);
    }
  }
  //posledni radek prazdny
  for (j = 0; j < VGA_COLUMNS; j++) {
    addr = 0xA000 /*base*/ + (VGA_ROWS - 1) * VGA_STRIDE + j;
    val = 0;
    FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, addr, &val, 2, 1);
  }          
}

/*******************************************************************************
 * Nacteni fontu z flash, Inicializace
 ******************************************************************************/
void Font_Flash_FPGA(void)
{
  unsigned char buf[16];
  unsigned short bramAddr;
  int i, j, page, offset;

  term_send_str("Loading font from flash ... ");

  page = FLASH_USER_DATA_PAGE;
  offset = 0;

  bramAddr = 0x8000; //base addr
  for (i = 0; i < 128; i++) // 128 x 16B 
  {
    // Nacteni bloku z FLASH
    FLASH_ReadData((unsigned char *)&buf, 16, page, offset);

    //posun na dalsi blok
    offset += 16;
    if (offset >= PAGE_SIZE) {
       offset -= PAGE_SIZE;
       page += 1;
    }
    
    // Odeslani do FPGA
    for (j = 0; j < 16; j++) {
        //11 bitu data (0..2047) 5 bitu base
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, bramAddr, &(buf[j]), 2, 1);
        bramAddr++;
    }
    WDG_reset();
  }
  term_send_str_crlf("OK");
}

void VRAM_Init(void) {
  unsigned short i;
  unsigned char buf[2];

  term_send_str("Initializing VRAM ... ");

  VGA_Clear();
  VGA_TextOut("==========================================================\n");
  VGA_TextOut("= FITkit - tabulka znaku                                 =\n");
  VGA_TextOut("==========================================================\n");
  VGA_TextOut(" Prvni radek\n Druhy radek");

  #define TABX 2
  #define TABY 6

  buf[1] = 0;
  for (i = 0; i < 16; i++) 
  {
    VGA_GotoXY(TABX + 5 + (i << 1), TABY); //radek
    buf[0] = digit2chr(i);
    VGA_TextOut(buf);
    VGA_GotoXY(TABX + 5 + (i << 1), TABY + 1);
    VGA_TextOut("\x94\x94");

    if (i < 8) 
    {
      VGA_GotoXY(TABX, TABY + 2 + i); //sloupec
      buf[0] = digit2chr(i + 2);
      VGA_TextOut(buf);
      VGA_TextOut("X \x83");
    }
  }
  VGA_GotoXY(TABX, TABY + 1);
  VGA_TextOut("\x94\x94\x94\x95\x94");
  VGA_GotoXY(TABX + 3, TABY);
  VGA_TextOut("\x83");

  for (i = 0; i < 128; i++)
  {
     WDG_reset();
     VGA_GotoXY(TABX + 5 + ((i & 0x0F) << 1), TABY + 2 + (i >> 4));
     buf[0] = i+32;
     VGA_TextOut(buf);
  }

  VGA_GotoXY(0, TABY + 12);
  
  term_send_str_crlf("OK");
}
