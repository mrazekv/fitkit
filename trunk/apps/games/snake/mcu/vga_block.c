/*******************************************************************************
   vga_block.c: VGA - 80x60 blocks
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
              Karel Slany    <slany AT fit.vutbr.cz>

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

#include "vga_block.h"


unsigned short vga_pos = 0;

//Vymazani video RAM
void VGA_Clear(void) {
  unsigned short i, j;
  unsigned char c;        
  vga_pos = 0;
  for (i=0;i<VGA_ROWS;i++) {
     for (j = 0; j<VGA_COLUMNS; j++) {
        c = 0;
        // Odeslani dat do FPGA (2 byty adresy, 1 byte dat)
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_VRAM/*base*/ + i*VGA_STRIDE + j, &c, 2, 1);
     }
  }
}

//Zmena pozice
void VGA_GotoXY(unsigned char x, unsigned char y) {
  if ((x > VGA_COLUMNS) || (y > VGA_ROWS)) return;
  vga_pos = y*VGA_STRIDE + x;
}

//Zapsani hodnoty
void VGA_SetPixel(char val) {
  FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_VRAM /*base*/ + vga_pos, &val, 2, 1);
}

//Zapsani hodnoty na souradnici X,Y
void VGA_SetPixelXY(unsigned char x, unsigned char y, char val) {
  unsigned short vga_pos = y*VGA_STRIDE + x;
  FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_VRAM /*base*/ + vga_pos, &val, 2, 1);
}

//Inicializace VRAM
void VRAM_Init(void) {
  short i;
  term_send_str_crlf("Initializing VRAM");
  VGA_Clear();
  
  //bily ramecek
  for (i=0;i<VGA_COLUMNS;i++) {
      VGA_SetPixelXY(i,0, 1);
      VGA_SetPixelXY(i,VGA_ROWS-1, 1);
  }
  for (i=0;i<VGA_ROWS;i++) {
      VGA_SetPixelXY(0,i, 1);
      VGA_SetPixelXY(VGA_COLUMNS-1,i, 1);
  }

  term_send_str_crlf("Ok");
}
