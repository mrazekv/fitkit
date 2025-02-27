/*******************************************************************************
   main.c: main for blockram demo application
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

#define BRAM_BASE 0


/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" BRAM CLEAR ...... vynuluje obsah pameti BRAM");
  term_send_str_crlf(" BRAM READ  ...... cteni obsahu pameti BRAM");
  term_send_str_crlf(" BRAM WRITE ...... zapis hodnot do pameti BRAM");
}


/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  unsigned short ofs, val, cnt;

  if (strcmp10(cmd_ucase, "BRAM CLEAR"))
  {
    for (ofs=0; ofs < 1024; ofs++)
    {
      val = 0;
      //zapis 16 bitove hodnoty val na adresu BRAM_BASE + offset ofs
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BRAM_BASE+ofs, (unsigned char *)&val, 2, 2);
    }
  }
  else if (strcmp10(cmd_ucase, "BRAM WRITE"))
  {
    for (ofs=0; ofs < 1024; ofs++)
    {
      val = ofs;
      //zapis 16 bitove hodnoty val na adresu BRAM_BASE + offset ofs
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BRAM_BASE+ofs, (unsigned char *)&val, 2, 2);
    }
  }
  else if (strcmp9(cmd_ucase, "BRAM READ"))
  {
    cnt = 8;
    for (ofs=0; ofs < 1024; ofs++)
    {
      if (cnt==8)
      {
        term_send_crlf();
        term_send_char(' ');
        cnt=0;
      }
      cnt++;

      //cteni 16 bitove hodnoty z adresy BRAM_BASE + offset ofs
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, BRAM_BASE+ofs, (unsigned char *)&val, 2, 2);
      term_send_hex(val);
      term_send_char(' ');
    }
  }
  else
  {
    return (CMD_UNKNOWN);
  }
  return (USER_COMMAND);
}



/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
//INLINE INT INIT_MCU(VOID)
int main(void)
{
  unsigned int counter = 0;

  initialize_hardware();

  set_led_d6(1);  // rozsviceni D6
  set_led_d5(1);  // rozsviceni D5

  while (1) 
  {
    delay_ms(1);  //zpozdeni 1ms

    counter++;
    if (counter == 500) 
    {
      flip_led_d6(); //invertovat LED
      counter = 0;
    }

    terminal_idle();  // obsluha terminalu
  }

}
