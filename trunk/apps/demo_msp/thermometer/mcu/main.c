/*******************************************************************************
   main: Zobrazeni teploty cidla v MCU na displeji.
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Vojtech Cizinsky <xcizin01 AT stud.fit.vutbr.cz>

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
#include <stdio.h>
#include <lcd/display.h>
#include <thermometer/thermometer.h>

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
    term_send_str_crlf("Zobrazeni teploty cidla v MCU na displeji.");
}

/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) 
{
   return (CMD_UNKNOWN);
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized() 
{
   LCD_init();
   LCD_write_string("Thermometer ...");
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
   short counter = 0;
   int temp;                             // udaj o teplote
   char result[20];                      // textovy retezec zobrazeny na displeji

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

         temp = thermometer_gettemp();                  // zjisteni aktualni teploty
         sprintf (result, "%02d.%01d'C", temp/10, temp%10);
         LCD_write_string(result);        // zobrazeni teploty
         term_send_str_crlf(result);

         counter = 0;
      }

      terminal_idle();  // obsluha terminalu
   }

}
