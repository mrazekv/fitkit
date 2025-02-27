/*******************************************************************************
   main.c: DISPLAY CZ
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Pavel Bartos <xbarto41 AT stud.fit.vutbr.cz>

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
#include <lcd_cz/display_cz.h>


/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help") 
 * systemoveho helpu  
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" INIT             inicializuje LCD");
  term_send_str_crlf(" CLEAR            smaze LCD");
  term_send_str_crlf(" STRING <string>  vypise na LCD retezec (latin-2)");
  term_send_str_crlf(" CHAR <char>      vypise na LCD znak");
  term_send_str_crlf(" CHR <ord>        vypise na LCD znak odpovidajici zadane ordinalni hodnote");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp4(cmd_ucase, "INIT"))
  {
    lcd_init();
    return USER_COMMAND;
  }
  if (strcmp5(cmd_ucase, "CLEAR"))
  {
    lcd_clear();
    return USER_COMMAND;
  }
  if (strcmp6(cmd_ucase, "STRING"))
  {
    lcd_string((unsigned char *)&cmd[7]);
    return USER_COMMAND;
  }
  if (strcmp4(cmd_ucase, "CHAR"))
  {
    lcd_char(cmd[5]);
    return USER_COMMAND;
  }
  if (strcmp3(cmd_ucase, "CHR"))
  {
    unsigned char ch = str2long((unsigned char *)&cmd[4]);
    lcd_char(ch);
    return USER_COMMAND;
  }

  return (CMD_UNKNOWN);
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized() {
  lcd_init();
}



/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
//inline int INIT_MCU(void)                     
int main(void) {      
  unsigned int cnt;

  initialize_hardware();

  lcd_init();
  // inicializace LED, LED0 vystup,
  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

  //latin-2
  lcd_string("LCD CZ: ì¹èø¾ýáí");

  cnt = 0;
       
  while (1) {
    delay_ms(10);
    cnt++;
    if (cnt > 50) {
      cnt = 0;
      flip_led_d6();                      // negace portu na ktere je LED
    }
    terminal_idle();                   // obsluha terminalu 
  }
}
