/*******************************************************************************
   picocpu.c: PicoCPU support unit
   Copyright (C) 2011 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
              Èapka Ladislav <xcapka01 AT stud.fit.vutbr.cz>
             

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
#include "picocpu.h"

/*******************************************************************************
 * Nacteni programu z FLASH a zapsani do BRAM v FPGA
 ******************************************************************************/
void PicoCPU_program_from_flash(void)
{
  unsigned char buf[7];
  unsigned char instr[3];
  long c_instr;
  long addr;
  
  int i, j;
  unsigned char c4;

  term_send_str_crlf("Loading SW from flash ");

  addr = FLASH_PageToAddr(PICOROM_PAGE); //base
  for (i = 0; i<1024; i++) // 1024 radku (instrukci) 
  {
    // Nacteni bloku z FLASH
    FLASH_ReadDataA((unsigned char *)&buf, 7, addr);
    addr += 7;
          
    // Radek v HEX souboru KCMPS3 = 20-bit hex hodnota jako retezec = 1 instrukce
    c_instr = 0;
    for (j = 0; j<5; j++)
    {
      c4 = buf[j];
      if (c4 > 96) c4 -= 87;
      else if (c4 > 64) c4 -= 55;
      else c4 -= 48;
      c_instr = (c_instr << 4) + (c4 & 0x0F);   
    }
    
    instr[0] = (c_instr >> 16) & 255;
    instr[1] = (c_instr >> 8) & 255;
    instr[2] = (c_instr) & 255;

    // Odeslani instrukce do FPGA
    FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_SWRAM /*base*/ + i, &(instr[0]), 2, 3);

  }
  term_send_str_crlf("Ok");
}

/*******************************************************************************
 * Upload HEX souboru do BRAM v FPGA
 ******************************************************************************/

int c_ofs;
long c_instr;
long c_addr;

char processBlock(unsigned short block, unsigned char* buf, unsigned short len) {
  int i = 0;
  unsigned char c4;
  unsigned char instr[3];

  for (i=0; i < len; i++) {
     c4 = buf[i];

     if (c4 > 96) c4 -= 87;
     else if (c4 > 64) c4 -= 55;
     else c4 -= 48;
     c_instr = (c_instr << 4) + (c4 & 0x0F);   

     if (c_ofs == 4) {
        instr[0] = (c_instr >> 16) & 255;
        instr[1] = (c_instr >> 8) & 255;
        instr[2] = (c_instr) & 255;

        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, c_addr, &(instr[0]), 2, 3);
        c_addr++;
        c_ofs++;
     } else if (buf[i] == 0x0A) {
        c_ofs = 0;
        c_instr = 0;
     } else {
        c_ofs++;
     }
  }

  return 1;
}

void PicoCPU_program(void)
{
  c_ofs = 0; c_instr = 0; c_addr = BASE_ADDR_SWRAM;
  ReceiveFile("Program pro PicoBlaze (7168B)", "*.hex", 7168, &processBlock);
}

/*******************************************************************************
 * Funkce pro obsluhu CPU
 ******************************************************************************/

void PicoCPU_disable(void)
{
  FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_CPUCONTROL, 0);
}

void PicoCPU_enable(void)
{
  FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_CPUCONTROL, 1);
}
