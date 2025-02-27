/*******************************************************************************
   user_main: main for application using keyboard with interrupt
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Zdenek Vasicek <vasicek AT stud.fit.vutbr.cz>

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

#define SERIAL_BASE_ADDR 0x90

/*******************************************************************************
 * Obsluha preruseni od seriove linky (prijem znaku)
*******************************************************************************/
void serial_handle_interrupt() {
   //prisel znak - precteme jej
   unsigned char data = FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ, SERIAL_BASE_ADDR, 0);
   //posleme prijaty znak zpet
   FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE, SERIAL_BASE_ADDR, data);
           
   term_send_hex(data);
   if ((data > 31) && (data < 128))
   {
      //je-li znak tisknutelny, zobrazime jej v terminalu
      term_send_str(" (");
      term_send_char((char)data);
      term_send_char(')');
   }
   term_send_char(' ');
}

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  fpga_interrupt_init(BIT0);
}

/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

/*******************************************************************************
 * Obsluha preruseni z FPGA
*******************************************************************************/
void fpga_interrupt_handler(unsigned char bits)
{
   if (bits & BIT0)
      serial_handle_interrupt(); 
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  initialize_hardware();

  set_led_d5(1);                       // rozsviceni D6

  while (1)
  {
    delay_ms(10);
    terminal_idle();                   // obsluha terminalu
  }         
}
