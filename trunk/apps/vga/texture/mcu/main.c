/*******************************************************************************
   main.c: main application for VGA demo
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): 
              Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>

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

#include <string.h>

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void){
  term_send_str_crlf(" FLASH W TEX......zapis textury do FLASH");
  term_send_str_crlf("   velikost 64x64 px, 1px = 2B BGR 333");
  term_send_str_crlf(" FLASH W MASK......zapis masky do FLASH");
  term_send_str_crlf("   velikost 32x32, indexy textury 0 az 3");
  term_send_str_crlf(" UPDATE TEX......nacteni textur do FPGA (BRAM)");
}


/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp(cmd_ucase, "FLASH W TEX") == 0)
  {
    FLASH_WriteFile(TEXTURE_PAGE,"Textura (8192B)","*.bin",32); //max 8291B
    return USER_COMMAND;
  }
  else if (strcmp(cmd_ucase, "FLASH W MASK") == 0)
  {
    FLASH_WriteFile(MASK_PAGE,"Maska (1024B)","*.bin",4); //max 1024B
    return USER_COMMAND;
  }
  else if (strcmp(cmd_ucase, "UPDATE TEX") == 0)
  {
    TEX_Flash_FPGA();
    return USER_COMMAND;
  }
  else
    return CMD_UNKNOWN;
}


/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized(){
  TEX_Flash_FPGA();
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
//INLINE INT INIT_MCU(VOID)
int main(void)
{
  unsigned int cnt = 0;

  initialize_hardware();

  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

  while (1)
  {
    delay_ms(10);
    cnt++;
    if (cnt > 50)
    {
      cnt = 0;
      if (Keyboard_FPGA() != 0)
        flip_led_d6();
    }
    terminal_idle();                   // obsluha terminalu
  }
}
