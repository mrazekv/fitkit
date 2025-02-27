/*******************************************************************************
   vga.c: VGA demo additional functions
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>

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

*******************************************************************************/

#include <fitkitlib.h>

#include "vga.h"

unsigned short pos_x = 100;
unsigned short pos_y = 380;

/*******************************************************************************
 * Vygeneruje a posle barvu do RAMB
 ******************************************************************************/
void TEX_Flash_FPGA(void)
{
  unsigned char buf[128];
  unsigned char data[2];
  int i, j, Clr;
  long addr;

  term_send_str("Loading texture from FLASH ");

  addr = FLASH_PageToAddr(TEXTURE_PAGE); //base
  for (i = 0; i < 64; i++) // 64x64 textura
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

      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, (i * 64) + j, &(data[0]), 2, 2);
    }
    WDG_reset();
  }

  term_send_str_crlf("OK");

  term_send_str("Loading mask from FLASH ");

  addr = FLASH_PageToAddr(MASK_PAGE); //base
  for (i = 0; i < 32; i++) // 32x32 maska
  {
    FLASH_ReadDataA((unsigned char *)&buf, 32, addr);
    addr += 32;

    for (j = 0; j < 32; j++)
    {
      Clr = (int)buf[j];

      data[0] = (unsigned char)((Clr >> 8) & 255);
      data[1] = (unsigned char)(Clr & 255);

      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 4096 + (i * 32) + j, &(data[0]), 2, 2);
    }
    WDG_reset();
  }

  term_send_str_crlf("OK");
}

unsigned short Keyboard_FPGA(void)
{
  unsigned char data[4];
  unsigned short keyst;

  // Pole dat k odeslani funkci FPGA_SPI_RW_AN_DN
  data[0] = (unsigned char)((pos_y >> 8) & 255);
  data[1] = (unsigned char)(pos_y & 255);
  data[2] = (unsigned char)((pos_x >> 8) & 255);
  data[3] = (unsigned char)(pos_x & 255);

  // Odeslani/prijem bloku dat  
  FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE + SPI_FPGA_ENABLE_READ, 0x40, &(data[0]), 1, 4);
  
  // V poli jsou prijata data z FPGA
  keyst = ((unsigned short)(data[2]) << 8) + data[3];

  if (keyst & 2)   pos_x--;
  if (keyst & 512) pos_x++;
  if (keyst & 16)  pos_y--;
  if (keyst & 32)  pos_y++;

  return keyst;
}
