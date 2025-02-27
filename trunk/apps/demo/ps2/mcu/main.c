/*******************************************************************************
   main.c : PS/2 demo
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
#include <irq/fpga_interrupt.h>

#define BASE_ADDR_PS2 0x02
#define read_byte_ps2() FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ, BASE_ADDR_PS2, 0)
#define write_byte_ps2(byte) FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_PS2, byte)

volatile unsigned char ps2_buf;
volatile char ps2_buf_full;
volatile char ps2_readen;

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
    term_send_str_crlf("User help:");
    term_send_str_crlf(" KBLED  ...... led");
    term_send_str_crlf(" KBECHO ...... echo");
    term_send_str_crlf(" KBID   ...... ID");
    term_send_str_crlf(" KBEN   ...... enable");
    term_send_str_crlf(" KBDIS  ...... disable");
    term_send_str_crlf(" KBCMD XX .... send hexa command");
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized() 
{
  fpga_interrupt_init(BIT2);
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp5(cmd_ucase, "KBCMD")) {
    write_byte_ps2(hex2chr(cmd + 6)); 
    return (USER_COMMAND);
  }

  if (strcmp5(cmd_ucase, "KBLED")) {
     ps2_readen = 0;
     ps2_buf_full = 0;
     write_byte_ps2(0xED);
     term_send_str_crlf("KBLED wait for response");
     while (ps2_buf_full == 0) _NOP();
     if (ps2_buf == 0xFA) {
        ps2_buf_full = 0;
        term_send_str_crlf("KBLED ACK");
        write_byte_ps2(0x01);
     } else {
        term_send_str_crlf("KBLED NAK");
     }
     ps2_readen = 1;
     return (USER_COMMAND);
  } 

  if (strcmp6(cmd_ucase, "KBECHO")) {
     write_byte_ps2(0xEE);
     return (USER_COMMAND);
  } 

  if (strcmp4(cmd_ucase, "KBID")) {
     write_byte_ps2(0xF2);
     return (USER_COMMAND);
  } 

  if (strcmp4(cmd_ucase, "KBEN")) {
     write_byte_ps2(0xF4);
     return (USER_COMMAND);
  } 

  if (strcmp5(cmd_ucase, "KBDIS")) {
     write_byte_ps2(0xF5);
     return (USER_COMMAND);
  } 
  return (CMD_UNKNOWN);
}

/*******************************************************************************
 * Obsluha preruseni z FPGA
*******************************************************************************/
void fpga_interrupt_handler(unsigned char bits)
{
  if (bits & BIT2) {
     ps2_buf = read_byte_ps2();
     ps2_buf_full = 1;
  }
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  short counter = 0;
  char cnt;

  cnt = 0;
  ps2_buf_full = 0;
  ps2_readen = 1;

  initialize_hardware();

  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

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

      if ((ps2_readen == 1) && (ps2_buf_full == 1)) 
      {
         //Vypis prijatych znaku na terminal
         ps2_buf_full = 0;

         term_send_hex(ps2_buf);
         term_send_char(' ');

         cnt++;
         if (cnt > 15) 
         {
            term_send_crlf();
            cnt=0;
         }
      }
  }         
} 
