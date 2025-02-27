/*******************************************************************************
   main.c: PING server
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
#include <nrf24l01/nrf24l01.h>

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
 * systemoveho helpu
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf("RF PING server");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return USER_COMMAND;
}


/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
   term_send_str("NRF init ... ");
   if (NRF24L01_init() == 1) {
      term_send_str_crlf("OK");

      NRF_config(24, 1, "srv01", 1);
      term_send_str_crlf("Listening ...");

   } else {
      term_send_str_crlf("FAILED");
   }
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  unsigned char data;

  initialize_hardware();

  set_led_d5(0);
  set_led_d6(0);

  //PING server
  while (1) {

    if (NRF_dataReady()) {
       term_send_str_crlf("received data: ");
       do {
         data = NRF_getByte();
         term_send_hex(data);
         term_send_crlf();

         delay_ms(100); 

         NRF_setTXAddress("cli01");
         NRF_sendByte(data);

         term_send_str("send byte back .");
         while (NRF_isSending()) { 
            term_send_char('.'); 
            delay_ms(250); 
         }
         term_send_crlf();

       } while (!NRF_isRxEmpty());

       term_send_str_crlf("Listening ...");
    }
    terminal_idle();                       // obsluha terminalu
  }
}
