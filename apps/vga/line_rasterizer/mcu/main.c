/*******************************************************************************
   main.c: main application for VGA demo
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>

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

#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" GENPIC....vygeneruje barevny obrazek");
  term_send_str_crlf(" GENPIC2...vygeneruje bily obrazek");
  term_send_str_crlf(" GL........vygeneruje usecku");
}

unsigned int mrand(unsigned int lim)
{
  unsigned int res = rand();
  return res % lim;
}

char SetLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char clr)
{
  unsigned char data[5];
  
  // Out of range
  if (x1 >= 512) return -1;
  if (y1 >= 480) return -1;
  if (x2 >= 512) return -1;
  if (y2 >= 480) return -1;
  
  data[5] = x1 & 0xFF;                               // FPGA (7 downto 0)
  data[4] = ((x1 >> 8) & 0x01) + ((y1 << 1) & 0xFF); // FPGA (15 downto 8)
  data[3] = ((y1 >> 7) & 0x03) + ((x2 << 2) & 0xFF); // FPGA (23 downto 16)
  data[2] = ((x2 >> 6) & 0x07) + ((y2 << 3) & 0xFF); // FPGA (31 downto 24)
  data[1] = ((y2 >> 5) & 0xFF);                      // FPGA (39 downto 32)
  data[0] = clr;                                     // FPGA (47 downto 40)
 
  FPGA_SPI_RW_AN_DN(1, 0, &(data[0]), 1, 6);
 
  return 0;
}

void VGA_GENPIC(char id)
{
  int i;
  char res;

  switch (id)
  {
    case 0: 
      SetLine(0, 470, 511, 470, mrand(256));
    
      SetLine(256, 470, 256, 150, mrand(256));
      SetLine(256, 370, 156, 270, mrand(256));
      SetLine(256, 330, 356, 230, mrand(256));
  
      for (i = -75; i <= 75; i += 15)
      {
        SetLine(256, 150, 256 + i, 150 - 75, mrand(256));
        SetLine(256, 150, 256 + i, 150 + 75, mrand(256));
        SetLine(256, 150, 256 - 75, 150 - i, mrand(256));
        SetLine(256, 150, 256 + 75, 150 + i, mrand(256));
      }
      break;
  
    case 1:
      SetLine(0, 470, 511, 470, 255);
      
      SetLine(256, 470, 256, 150, 255);
      SetLine(256, 370, 156, 270, 255);
      SetLine(256, 330, 356, 230, 255);
      
      for (i = -75; i <= 75; i += 15)
      {
         SetLine(256, 150, 256 + i, 150 - 75, 255);
         SetLine(256, 150, 256 + i, 150 + 75, 255);
         SetLine(256, 150, 256 - 75, 150 - i, 255);
         SetLine(256, 150, 256 + 75, 150 + i, 255);
      }
      break;
   
    case 2:
      res = SetLine(mrand(480), mrand(480), mrand(480), mrand(480), mrand(256));
      if (res != 0)
        term_send_str("Err"); 
      break;
  }
}

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if ((strlen(cmd_ucase) == 2) && (strcmp2(cmd_ucase, "GL")))
  {
    VGA_GENPIC(2);
    return (USER_COMMAND);
  }
  else if ((strlen(cmd_ucase) == 7) && (strcmp7(cmd_ucase, "GENPIC2")))
  {
    VGA_GENPIC(1);
    return (USER_COMMAND);
  }
  else if ((strlen(cmd_ucase) == 6) && (strcmp6(cmd_ucase, "GENPIC")))
  {
    VGA_GENPIC(0);
    return (USER_COMMAND);
  }
  else
    return (CMD_UNKNOWN);
}



/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized(){
//  TEX_Flash_FPGA();
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
//INLINE INT INIT_MCU(VOID)
int main(void)
{
  unsigned int cnt = 0;

  initialize_hardware();
  srand(100);

  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

  while (1)
  {
    delay_ms(10);
    cnt++;
    if (cnt > 50) {
      cnt = 0;
      flip_led_d6();
    }
    terminal_idle();                   // obsluha terminalu
  }         
}
