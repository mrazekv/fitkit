/*******************************************************************************
   vga.c: VGA functions needed in 3D vector gfx demo
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Josef Strnadel <strnadel AT fit.vutbr.cz>

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

char SetTriangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, unsigned char clr, unsigned char cls)
{
  unsigned char data[8];

  // Out of range
  if (x1 >= 512) return -1;
  if (y1 >= 480) return -1;
  if (x2 >= 512) return -1;
  if (y2 >= 480) return -1;

  data[7] = x1 & 0xFF;                               // FPGA (7 downto 0)
  data[6] = ((x1 >> 8) & 0x01) + ((y1 << 1) & 0xFF); // FPGA (15 downto 8)
  data[5] = ((y1 >> 7) & 0x03) + ((x2 << 2) & 0xFF); // FPGA (23 downto 16)
  data[4] = ((x2 >> 6) & 0x07) + ((y2 << 3) & 0xFF); // FPGA (31 downto 24)
  data[3] = ((y2 >> 5) & 0x0F) + ((x3 << 4) & 0xFF); // FPGA (39 downto 32)
  data[2] = ((x3 >> 4) & 0x1F) + ((y3 << 5) & 0xFF); // FPGA (47 downto 40)
  data[1] = ((y3 >> 3) & 0x3F) + ((cls & 1) << 7);   // FPGA (55 downto 48)
  data[0] = clr;                                     // FPGA (63 downto 56)

  FPGA_SPI_RW_AN_DN(1, 0, &(data[0]), 1, 8);

  return 0;
}

void VGA_pixel(unsigned int x, unsigned int y, unsigned char color, unsigned char cls)
{
  SetTriangle(x, y, x-1, y, x, y-1, color, cls);
}

void VGA_line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color, unsigned char cls)
{
  SetTriangle(x1, y1, x2, y2, (x2+x1+2)/2, (y2+y1-2)/2, color, cls);
}

void VGA_clear(unsigned char color)
{
  SetTriangle(0, 0, 0, 10, 10, 0, color + 4, 1);
  //SetTriangle(0, 0, 0, 479, 511, 0, color, 0);
  //SetTriangle(0, 479, 511, 0, 511, 479, color, 0);
}
