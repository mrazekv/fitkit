/*******************************************************************************
   user_main: main for LED diode test
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology

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
#include <string.h>
#include <picocpu/picocpu.h>

unsigned int iter; //globalni promenna

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" FLASH W HEX ...... zapis programu ve formatu HEX pro HW-CPU");
  term_send_str_crlf("                    (soubor 7168 B vytvoreny KCPSM3.EXE)");
  term_send_str_crlf(" CPU STOP ...... zastaveni CPU");
  term_send_str_crlf(" CPU START ..... reset CPU");
  term_send_str_crlf(" CPU UPDATE ...... aktualizace programu (+autostart programu)");
  term_send_str_crlf(" PROG ...... naprogramovani a aktualizace");

}

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp(cmd_ucase, "FLASH W HEX") == 0)
  {
    FLASH_WriteFile(PICOROM_PAGE,"Program pro PicoBlaze (7168B)","*.hex",32); //max 8291B
    return USER_COMMAND;
  }
  else if (strcmp(cmd_ucase, "CPU STOP") == 0) {
    PicoCPU_disable();
    return (USER_COMMAND);
  }
  else if (strcmp(cmd_ucase, "CPU START") == 0) {
    PicoCPU_enable();
    return (USER_COMMAND);
  }
  else if (strcmp(cmd_ucase, "CPU UPDATE") == 0) {
    PicoCPU_disable();
    PicoCPU_program();
    PicoCPU_enable();
    return (USER_COMMAND);
  }
  else if (strcmp(cmd_ucase, "PROG") == 0)
  {
//    FLASH_WriteFile(PICOROM_PAGE,"Program pro PicoBlaze (7168B)","*.hex",32); //max 8291B
    PicoCPU_disable();
    PicoCPU_program();
//    PicoCPU_program_from_flash();
    PicoCPU_enable();
    return USER_COMMAND;
  }
  else
    return CMD_UNKNOWN;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized() {
  decode_user_cmd("PROG","PROG");
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
   short counter = 0;
   iter = 0;

   initialize_hardware();

   set_led_d6(1);  //rozsvitit LED D6
   set_led_d5(1);  //rozsvitit LED D5

   while (1) 
   {
         
      delay_ms(1);  //zpozdeni 1ms

      counter++;
      if (counter == 500) 
      {
         flip_led_d6(); //invertovat LED

         iter++;
         counter = 0;
      }

      terminal_idle();  // obsluha terminalu
   }

}


