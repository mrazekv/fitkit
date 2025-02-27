/*******************************************************************************
   main.c: Text mode demo
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


/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help") 
*******************************************************************************/
void print_user_help(void){
  term_send_str_crlf(" FLASH W FONT......zapis textury do FLASH, velikost 128*16=2048B");
  term_send_str_crlf(" UPDATE FONT......nacteni fontu do BRAM v FPGA");
  term_send_str_crlf(" VGA CLEAR ...... smazani videoRAM");
  term_send_str_crlf(" VGA TEXT \"text\" ...... zapis textu do videoRAM");
  term_send_str_crlf(" VGA CRLF ...... novy radek");
  term_send_str_crlf(" VGA SCROLL ...... posun obsahu o jeden radek nahoru");
}


/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) {
  if (strcmp(cmd_ucase, "FLASH W FONT") == 0) {
    FLASH_WriteFile(FLASH_USER_DATA_PAGE,"Font 128 znaku (16x8)","*.fnt",8); //max 2048 B, cca 8 stranek
    Font_Flash_FPGA(); //update
    return USER_COMMAND;
  }

  if (strcmp(cmd_ucase, "UPDATE FONT") == 0) {
    Font_Flash_FPGA();
    return USER_COMMAND;
  }
    
  if (strcmp4(cmd_ucase,"VGA ")) {
    if (strcmp5(cmd_ucase+4, "TEXT ")) {
      VGA_TextOut(cmd+9);
      //VGA_TextOut("\n");
      return USER_COMMAND;
    }

    if (strcmp(cmd_ucase+4, "CRLF")==0) {
      VGA_TextOut("\n");
      return USER_COMMAND;
    }

    if (strcmp(cmd_ucase+4, "SCROLL")==0) {
      VGA_ScrollUp();
      return USER_COMMAND;
    }

    if (strcmp(cmd_ucase+4, "CLEAR") == 0) {
      VGA_Clear();
      return USER_COMMAND;
    }
  }
  return CMD_UNKNOWN;
}



/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized(){
  Font_Flash_FPGA();
  VRAM_Init();
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)                     
{
  unsigned int cnt = 0;

  initialize_hardware();

  set_led_d6(1);     // rozsviceni D6
  set_led_d5(1);     // rozsviceni D5  

  while (1)
  {
    delay_ms(10);    //zpozdeni 1ms
    cnt++;
    if (cnt == 50) 
    {
      flip_led_d6(); //invertovat LED
      cnt = 0;
    }
    terminal_idle(); // obsluha terminalu
  }         
}
