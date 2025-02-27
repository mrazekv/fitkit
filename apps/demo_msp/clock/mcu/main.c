/*******************************************************************************
   main: interrupt service routine (ISR) of timer A0 utilized for LED control
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Josef Strnadel <strnadel AT stud.fit.vutbr.cz>

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
#include "user_defined.h"

#include <string.h>

unsigned char clk_h, clk_m, clk_s;
unsigned short flag_timer;

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" SET HH:MM:SS  ... zmena casu");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  unsigned char data[8];
  
  if (strcmp4(cmd_ucase, "SET ")) 
  {
    term_send_str("Hodiny byly nastaveny na: ");
    strcpy(data, get_data(cmd, 8));
    term_send_str_crlf(data);
    
    // konverze ascii retezce hh:mm:ss na hodnoty hh, mm, ss
    clk_h = (data[0] - 48) * 10 + (data[1] - 48);   // hh
    clk_m = (data[3] - 48) * 10 + (data[4] - 48);   // mm
    clk_s = (data[6] - 48) * 10 + (data[7] - 48);   // ss
  } 
  else 
  {
    return CMD_UNKNOWN;
  }
  return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  term_send_str_crlf("----------------------------------------------------");
  term_send_str_crlf("Aplikace je pripravena k behu.");
  term_send_str_crlf("Zadejte prosim 'help' pro napovedu.");
  term_send_str_crlf("----------------------------------------------------");
  
  LCD_init();                          // inicializuj LCD
  LCD_append_string("FITkit: clock");  // zobraz text na LCD
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0); // vypni kurzor
}

/*******************************************************************************
 * Zobrazeni casu (ve formatu HH:MM:SS) na LCD 
*******************************************************************************/
void display_clock() {
  LCD_send_cmd(LCD_SET_DDRAM_ADDR | LCD_SECOND_HALF_OFS, 0); // kurzor na druhou polovinu
  delay_ms(2);

  LCD_send_cmd((unsigned char)(clk_h / 10) + 48, 1); delay_ms(2);     // zobraz hh
  LCD_send_cmd((unsigned char)(clk_h % 10) + 48, 1); delay_ms(2); 
  LCD_send_cmd(0x3A, 1); delay_ms(2);                                 // :
  LCD_send_cmd((unsigned char)(clk_m / 10) + 48, 1); delay_ms(2);     // zobraz mm
  LCD_send_cmd((unsigned char)(clk_m % 10) + 48, 1); delay_ms(2);     
  LCD_send_cmd(0x3A, 1); delay_ms(2);                                 // :
  LCD_send_cmd((unsigned char)(clk_s / 10) + 48, 1); delay_ms(2);     // zobraz ss
  LCD_send_cmd((unsigned char)(clk_s % 10) + 48, 1); delay_ms(2);     
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/

int main(void)
{
  flag_timer = 0;
  initialize_hardware();
  WDTCTL = WDTPW + WDTHOLD;                // zastav watchdog

  P1DIR |= 0x01;                            // nastav P1.0 na vystup (budeme ovladat zelenou LED D5)

  CCTL0 = CCIE;                             // povol preruseni pro casovac (rezim vystupni komparace) 
  CCR0 = 0x8000;                            // nastav po kolika ticich (32768 = 0x8000, tj. za 1 s) ma dojit k preruseni
  TACTL = TASSEL_1 + MC_2;                  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

  P1OUT ^= 0x01;                            // inicializuj P1.0 na 1 (D5 zhasnuta)

  clk_h = 0;                                // inicializuj hodiny
  clk_m = 0;                                // inicializuj minuty
  clk_s = 0;                                // inicializuj sekundy

  while (1)
  {
    terminal_idle();                       // obsluha terminalu
    if (flag_timer > 0) {
      display_clock();                    // zobraz aktualizovane hh:mm:ss
      flag_timer--;
    }
  }                                  
}

/*******************************************************************************
 * Obsluha preruseni casovace timer A0
*******************************************************************************/
interrupt (TIMERA0_VECTOR) Timer_A (void)
{
  P1OUT ^= 0x01;      // invertuj bit P1.0 (kazdou 1/2 s)
  CCR0 += 0x8000;     // nastav po kolika ticich (32768 = 0x8000, tj. za 1 s) ma dojit k dalsimu preruseni

  clk_s++; 

  if(clk_s==60) { clk_m++; clk_s=0; }     // kazdou 60. sekundu inkrementuj minuty
  if(clk_m==60) { clk_h++; clk_m=0; }     // kazdou 60. minutu inkrementuj hodiny
  if(clk_h==24)                         // kazdych 24 hodin nuluj h,m,s
  {
    clk_h = clk_m = clk_s = 0;
  }

  flag_timer += 1;
}
