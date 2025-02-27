/*******************************************************************************
   main: main for display
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jan Markovic <xmarko04 AT stud.fit.vutbr.cz>
              Zdenek Vasicek <vasicek AT fit.vutbr.cz>

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
#include <lcd/display.h>

char rotate_en;

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" CLEAR   ... smazani displeje");
  term_send_str_crlf(" CHAR X  ... zapis znaku X na displej a posun kurzoru");
  term_send_str_crlf(" STRING string ... zapis retezce na displej");
  term_send_str_crlf(" ROTATE     ... spusti/pozastavi rotaci retezce na displeji");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  int data;

  if (strcmp5(cmd_ucase, "CLEAR"))  // Vymazani displaye
  {
    LCD_clear();
  }
  else if (strcmp4(cmd_ucase, "TEST"))
  {
    LCD_clear();
    LCD_append_char('A');
    for (data=0;data<10;data++)
      LCD_append_char(48+data);
    for (data=0;data<5;data++)
      LCD_append_char(48+data);
    LCD_append_char('B');
    for (data=0;data<10;data++)
      LCD_append_char(48+data);
    for (data=0;data<5;data++)
      LCD_append_char(48+data);
    LCD_append_char('C');
    for (data=0;data<10;data++)
      LCD_append_char(48+data);
    for (data=0;data<5;data++)
      LCD_append_char(48+data);

  }
  else if (strcmp6(cmd_ucase, "ROTATE"))
  {
    if (rotate_en == 0)
    {
      rotate_en = 1;
      term_send_str("Rotace: povolena");
    }
    else
    {
      rotate_en = 0;
      term_send_str("Rotace: zakazana");
    }
  }
  else if (strcmp5(cmd_ucase, "CHAR "))
  {
    data = cmd[5];
    term_send_str("Poslano na displej: 0x");
    term_send_hex(data);
    term_send_crlf();

    LCD_append_char(data);
  }
  else if (strcmp7(cmd_ucase, "STRING "))
  {    // Zapis znaku na Display
    term_send_str("Poslano na displej: ");
    term_send_str((unsigned char *)&cmd[7]);
    term_send_crlf();

    LCD_append_string((unsigned char *)&cmd[7]);
  }
  else
  {
    return (CMD_UNKNOWN);
  }
  return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/

unsigned char charmap1[8] = {0x1F, 0x05, 0x01, 0x1C, 0x00, 0x04, 0x1C, 0x04}; //FIT
unsigned char charmap2[8] = {0x1F, 0x04, 0x0A, 0x11, 0x1E, 0x01, 0x1F, 0x01}; //KIT

void fpga_initialized()
{
  LCD_init();
  LCD_load_char(1, charmap1);
  LCD_load_char(2, charmap2);
  LCD_append_char('\x2');
  LCD_append_char('\x1');
  LCD_append_string(" Display");
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  unsigned int cnt = 0;
  rotate_en = 0;

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
      flip_led_d6();                  // negace portu na ktere je LED
      if (rotate_en == 1) 
        LCD_rotate();
    }

    terminal_idle();                  // obsluha terminalu
  } 

}
