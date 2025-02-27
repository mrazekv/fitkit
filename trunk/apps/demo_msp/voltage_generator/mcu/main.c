/*******************************************************************************
   main: main for 'demo_msp_voltage_generator' app
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ---------------------------------------------------------------------------- 
 */

const int mV_MIN = 0;
const int mV_MAX = 2500;

int mili_volts = 2500;          // hodnota mV
unsigned char str_tmp[MAX_COMMAND_LEN+1];
void get_data(unsigned char *str, int from);

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf("Napoveda pro generator napeti:");
  term_send_str_crlf(" mV x ... zmena amplitudy napeti na x [mV]");
  term_send_str("      x je mozno zadavat v rozmezi ");
  sprintf(str_tmp, "%d (mV_MIN) az %d (mV_MAX)", mV_MIN, mV_MAX);
  term_send_str_crlf(str_tmp);
  term_send_str_crlf("");
  term_send_str_crlf("Napeti je meritelne mezi vyvodem 31 (konektoru JP9) a zemi FITkitu");
}

/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *UString, char *String)
{	 
  if (strlen(UString)>=4)  // volba napeti [mV]
	{     
	   if((UString[0] == 'M') && (UString[1] == 'V') && (UString[2] == ' '))
	   {
        get_data(String, 3);  // do str_tmp ulozi UString od 4. znaku dal  
    	  mili_volts = atoi(str_tmp);

    	  if(mili_volts < mV_MIN)
    	  {
          term_send_str("Napeti zadano pod dolni mez napeti a proto bylo prenastaveno na jeji hodnotu (mV_MIN = ");
          sprintf(str_tmp, "%d [mV])", mV_MIN);
          term_send_str_crlf(str_tmp);
          mili_volts = mV_MIN;
        }
        else if(mili_volts > mV_MAX)
    	  {
          term_send_str("Napeti zadano nad horni mez napeti a proto bylo prenastaveno na jeji hodnotu (mV_MAX = ");
          sprintf(str_tmp, "%d [mV])", mV_MAX);
          term_send_str_crlf(str_tmp);
          mili_volts = mV_MAX;
        }
        
        sprintf(str_tmp, "Napeti: %d mV", mili_volts);
        LCD_write_string(str_tmp);
        term_send_str_crlf(str_tmp);
        
        DAC12_0DAT = 4095*((double)mili_volts/(mV_MAX - mV_MIN));   // preved mV na 12-bitovy vzorek
    }
    else { return (CMD_UNKNOWN); }
  }
	else { return (CMD_UNKNOWN); }

  return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
   LCD_init();
   LCD_append_string("Volt-generator");
   LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);  // vypni kurzor

  term_send_str_crlf("Napeti, ktere chcete generovat, nastavte pomoci prikazu mV (viz help).");
  term_send_str_crlf("Napeti je meritelne mezi vyvodem 31 (konektoru JP9) a zemi FITkitu");
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  initialize_hardware();
  WDTCTL = WDTPW + WDTHOLD;	 // zastav watchdog

  // nastaveni DAC
  ADC12CTL0 |= 0x0060;    // nastaveni referencniho napeti z ADC na 2,5 V
  DAC12_0CTL |= 0x0060;   // nastaveni kontrolniho registru DAC (12-bitovy rezim, medium speed)
  DAC12_0CTL |= 0x100;    // referencni napeti nasobit 1x
  DAC12_0DAT = 0;         // vynulovani vystupni hodnoty DAC

  // hlavni smycka
  while (1)
	{
    terminal_idle();      // obsluha terminalu
  }
}

/*******************************************************************************
 * Funkce ulozi do str_tmp retezec str od znaku from do konce
*******************************************************************************/
void get_data(unsigned char *str, int from)
{
	int i, j = 0;
	for (i = from; i < strlen(str); i++){	str_tmp[j++] = str[i]; }
	str_tmp[j] = 0;
}

