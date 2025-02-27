/*******************************************************************************
   main: Zobrazeni teploty cidla v MCU na displeji.
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Ladislav Sorcik <ladislav.sorcik@gmail.com>
              Karel Slany <slany AT fit.vutbr.cz>

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
#include <string.h>
#include <lcd/display.h>

#include "one_wire.h"


/*******************************************************************************
 * Prints user help (help command)
*******************************************************************************/
void print_user_help(void)
{
    term_send_str_crlf("DS18B20 temeperature measurement.");
}


/*******************************************************************************
 * User command decoding and execution
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) 
{
   return (CMD_UNKNOWN);
}


/*******************************************************************************
 * Peripheral/component initialization
*******************************************************************************/
void fpga_initialized() 
{
   LCD_init();
   LCD_write_string("Thermometer ...");
}


/*******************************************************************************
 * Appends a decadic number with at least digits characters, leading with fill
*******************************************************************************/
void LCD_append_long(unsigned long number, unsigned char digits, char fill)
{
  char buf[10];
  unsigned char length;
  long2str(number, buf, 10);
  length = strlen(buf);
  if (length < digits) {
    digits -= length;
    for (length = 0; length < digits; length++) {
      LCD_append_char(fill);
    }
  }
  LCD_append_string(buf);
}


/*******************************************************************************
 * Reads temperature from device which is the only device on the bus
*******************************************************************************/
unsigned int ds_get_temperature_single_dev(void)
{
  unsigned int temperature = 0;

  ow_reset();
  ow_write_byte(0xCC);      // skip ROM - no need to send device id
                            // because there is only one
  ow_write_byte(0x44);      // start temperature AD conversion
  while (!ow_read_bit()) {} // wait for conversion end
  ow_reset();
  ow_write_byte(0xCC);      // skip ROM - no need to send device id
  ow_write_byte(0xBE);      // read
  //((char*) & temperature)[1] = ow_read_byte();
  //((char*) & temperature)[0] = ow_read_byte();
  temperature = ow_read_byte();
  temperature |= ((unsigned int) ow_read_byte()) << 8;
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  return temperature;
}


/*******************************************************************************
 * Tells all the devices on the bus to start temperature measurement
*******************************************************************************/
void ds_measure_all(void)
{
  ow_reset();
  ow_write_byte(0xCC);      // skip ROM - no need to send device id
                            // because all are going to measure
  ow_write_byte(0x44);      // start temperature AD conversion
  while (!ow_read_bit()) {} // wait for conversion end
}


/*******************************************************************************
 * Reads temperature from a selected device
*******************************************************************************/
unsigned int ds_get_temperature_selected(unsigned char idx)
{
  unsigned int temperature = 0;

  ow_reset();
  ow_send_match_rom(idx);   // select by index
  ow_write_byte(0xBE);      // read
  temperature = ow_read_byte();
  temperature |= ((unsigned int) ow_read_byte()) << 8;
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  ow_read_byte();
  return temperature;
}


/*******************************************************************************
 * Main function
*******************************************************************************/
int main(void)
{
  short cntr = 0;
  signed int temperature, whole, fractional;
  unsigned long i;

  initialize_hardware();

  set_led_d6(0);

  ow_find_devices();

  LCD_write_string("Found ");
  LCD_append_long(num_ROMs, 2, '0');
  LCD_append_string(" devs");
  delay_ms(1000);

  while (1) 
  {
    ds_measure_all();
    for (i = 0; i < num_ROMs; i++) {
      temperature = ds_get_temperature_selected(i);
      LCD_write_string("dev");
      LCD_append_long(i, 1, '0');
      LCD_append_string(":  ");
      whole = temperature >> 4;
      fractional = 0;
      if (temperature >= 0)
      {
        //if (temperature & 0x0001) fractional +=  625;
        //if (temperature & 0x0002) fractional += 1250;
        //if (temperature & 0x0004) fractional += 2500;
        if (temperature & 0x0008) fractional += 5000;
        LCD_append_string(" ");
        LCD_append_long(whole, 3, ' ');
        LCD_append_char('.');
        LCD_append_long(fractional, 4, '0');
      }
      else
      {
        if (temperature & 0x000f)
        {
          whole += 1;
          fractional = 10000;
          //if (temperature & 0x0001) fractional -=  625;
          //if (temperature & 0x0002) fractional -= 1250;
          //if (temperature & 0x0004) fractional -= 2500;
          if (temperature & 0x0008) fractional -= 5000;
        } 
        whole *= -1;
        LCD_append_string(" ");
        LCD_append_long(whole, 3, ' ');
        LCD_append_char('.');
        LCD_append_long(fractional, 4, '0');
      }
    
      flip_led_d6();
      delay_ms(1000);
    }
     
  }

}
