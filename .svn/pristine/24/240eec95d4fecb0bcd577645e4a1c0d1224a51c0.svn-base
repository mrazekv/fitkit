/*******************************************************************************
   main: DS18B20
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
#include <lcd/display.h>

int onewire_reset() 
{
   unsigned char val[2];
   val[0] =  0x00;
   val[1] =  0x00;
   FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0xA0, (unsigned char *)&val, 1, 2);

   //BUSY
   for (;;) {
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0xA0, (unsigned char *)&val, 1, 2);
      if ((val[0] & 0x8) != 0x8)
         break;
   }
   return ((val[0] & 0x80) == 0x80);
}

void onewire_write(unsigned char data) 
{
   unsigned char val[2];
   val[0] =  0x01;
   val[1] =  data;
   FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0xA0, (unsigned char *)&val, 1, 2);

   //BUSY
   for (;;) {
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0xA0, (unsigned char *)&val, 1, 2);
      if ((val[0] & 0x8) != 0x8)
         break;
   }
}

unsigned char onewire_read() 
{
   unsigned char val[2];
   val[0] =  0x02;
   val[1] =  0x00;
   FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0xA0, (unsigned char *)&val, 1, 2);

   //BUSY
   for (;;) {
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0xA0, (unsigned char *)&val, 1, 2);
      if ((val[0] & 0x8) != 0x8)
         break;
   }

   //FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0xA0, (unsigned char *)&val, 1, 2);
   return val[1];
}

void LCD_append_int(val) 
{
  unsigned char buf[10];
  long2str(val, (unsigned char *)&buf, 10);
  LCD_append_string(buf);
}

/*******************************************************************************
 * Mereni teploty
 * --------------------------------------------------- 
 * Vraci pocet milisekund stravenych merenim
*******************************************************************************/
unsigned short measuretemp() 
{
  unsigned char val[10], k;
  unsigned short temp, tempf, dly = 0;

  //Detaily viz 
  //  http://datasheets.maxim-ic.com/en/ds/DS18B20.pdf

  if (!onewire_reset())
  {
     term_send_str_crlf("DS18B20 is not connected");
     return 0;
  }

  onewire_write(0xCC);
  onewire_write(0x44);

  //Cekame dokud neni prevod teploty dokoncen
  while (onewire_read() == 0) {
       delay_ms(1);
       dly++;

       if (dly == 1000) {
          term_send_str_crlf("DS18B20 busy timeout");
          return dly;
       }
  }

  //Vycteme namerenou hodnotu (obsah scratch pad memory)
  onewire_reset();
  onewire_write(0xCC);
  onewire_write(0xBE);
  delay_ms(1);
  
  for (k=0;k<9;k++) 
  {
    val[k] = onewire_read();
  }

#ifdef DEBUG
  term_send_str("ScratchPAD data:");
  for (k=0;k<9;k++) 
    term_send_str(" 0x");
    term_send_hex(val[k]);
  }
  term_send_crlf();
#endif

  temp = ((val[1] & 0x7) << 4) + (val[0] >> 4);
  tempf = 0;
  if (val[0] & 0x8) tempf += 500;
  if (val[0] & 0x4) tempf += 250;
  if (val[0] & 0x2) tempf += 125;
  if (val[0] & 0x1) tempf +=  60;
  
  LCD_write_string("Temp: ");
  LCD_append_int(temp);
  LCD_append_char('.');
  
  term_send_num(temp);
  term_send_char('.');
  if (tempf < 100) {
      term_send_char('0');
      LCD_append_char('0');
  }
  
  if (tempf < 10) {
      term_send_char('0');
      LCD_append_char('0');
  }
  term_send_num(tempf);
  term_send_str("'C");
  term_send_crlf();
  
  LCD_append_int(tempf);
  LCD_append_char(0xDF);

  return dly;
}

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" OW_RST    ... reset");
  term_send_str_crlf(" OW_WR XX  ... zapis dat XX");
  term_send_str_crlf(" OW_RD     ... cteni dat");
  term_send_str_crlf(" OW_TEMP   ... cteni teploty");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
   unsigned char val[10], data;

   if (strcmp5(cmd_ucase, "CLEAR")) { // Vymazani displaye
      LCD_clear();
   } else if (strcmp7(cmd_ucase, "OW_TEMP")) {
      measuretemp();    
   } else if (strcmp6(cmd_ucase, "OW_RST")) {
      term_send_str_crlf("OW Reset");
      onewire_reset();
   } else if (strcmp5(cmd_ucase, "OW_RD")) {
      term_send_str_crlf("OW Read: 0x");
      term_send_hex(onewire_read());
      term_send_crlf();
   } else if (strcmp5(cmd_ucase, "OW_WR")) {
      term_send_str_crlf("OW Write");
      data = hex2chr(cmd_ucase + 6);
      term_send_str("Write: 0x");
      term_send_hex(data);
      term_send_crlf();
      onewire_write(data);
   } else {
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0xA0, (unsigned char *)&val, 1, 2);
      term_send_str("Read: 0x");
      term_send_hex(val[0]);
      term_send_str(" 0x");
      term_send_hex(val[1]);
      term_send_crlf();
      return (CMD_UNKNOWN);
   }

   delay_ms(2);

   FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0xA0, (unsigned char *)&val, 1, 2);

   term_send_str("Read: 0x");
   term_send_hex(val[0]);
   term_send_str(" 0x");
   term_send_hex(val[1]);
   term_send_crlf();

   return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/

void fpga_initialized()
{
  LCD_init();
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  unsigned short cnt = 0;

  initialize_hardware();

  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

  while (1)
  {
    delay_ms(1);

    cnt++;
    if (cnt > 1000)                   //kazdou sekundu se provede mereni
    {
      cnt = 0;
      flip_led_d6();                  // negace portu na ktere je LED

      cnt = measuretemp();
    }

    terminal_idle();                  // obsluha terminalu
  } 

}
