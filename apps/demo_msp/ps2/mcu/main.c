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

//#define DEBUG_PS2
//#define DEBUG_PS2_IRQ

#include <fitkitlib.h>
#include <ps2/ps2_keyb.h>
#include <irq/fpga_interrupt.h> 

#include <ctype.h>

void print_key(unsigned char key);

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
 * systemoveho helpu
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" INIT             inicializuje PS/2 klavesnici");
  term_send_str_crlf(" NUM {0|1}        nastavi LED Num lock");
  term_send_str_crlf(" CAPS {0|1}       nastavi LED Caps lock");
  term_send_str_crlf(" SCROLL {0|1}     nastavi LED Scroll lock");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp4(cmd_ucase, "INIT"))
  {
    keyboard_ps2_init(&print_key);
    return USER_COMMAND;
  }
  if (strcmp3(cmd_ucase, "NUM"))
  {
    if (cmd[4]=='1')
      ps2_set_leds(ps2_get_leds() | (2));
    else if (cmd[4]=='0')
      ps2_set_leds(ps2_get_leds() & ~(2));
    else
      return CMD_UNKNOWN;
    return USER_COMMAND;
  }
  if (strcmp4(cmd_ucase, "CAPS"))
  {
    if (cmd[5]=='1')
      ps2_set_leds(ps2_get_leds() | (4));
    else if (cmd[5]=='0')
      ps2_set_leds(ps2_get_leds() & ~(4));
    else
      return CMD_UNKNOWN;
    return USER_COMMAND;
  }
  if (strcmp5(cmd_ucase, "SCROLL"))
  {
    if (cmd[7]=='1')
      ps2_set_leds(ps2_get_leds() | (1));
    else if (cmd[7]=='0')
      ps2_set_leds(ps2_get_leds() & ~(1));
    else
      return CMD_UNKNOWN;
    return USER_COMMAND;
  }
  return (CMD_UNKNOWN);
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  fpga_interrupt_init(BIT2);   
  keyboard_ps2_init(&print_key);
}

/*******************************************************************************
 * Obsluha preruseni z FPGA
*******************************************************************************/
void fpga_interrupt_handler(unsigned char bits)
{
  if (bits & BIT2) {
    #ifdef DEBUG_PS2_IRQ
      term_send_str_crlf("***DEBUG: PS2 IRQ");
    #endif
    keyboard_ps2_FM(); //obslouzeni preruseni
  }
} 

void print_key(unsigned char key)
{
  if (isprint(key))
  {
    term_send_str("PS2: char: '");
    term_send_char(key);
    term_send_str("' hex: 0x");
    term_send_hex(key);
    term_send_crlf();
  }
  else
  {
    term_send_str("PS2: hex: 0x");
    term_send_hex(key);
    term_send_crlf();
  }
}



/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
//inline int INIT_MCU(void)
int main(void) {
  unsigned int cnt;

  initialize_hardware();
   
  // inicializace LED, LED0 vystup,
  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

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
