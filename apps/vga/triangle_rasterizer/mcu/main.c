/*******************************************************************************
   main.c: main application for VGA demo
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): 
              Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>

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
#include "data.h"

#include <stdlib.h>
#include <string.h>

unsigned char Animate = 0;
unsigned char AnimateCnt = 0;
unsigned int  centerX = 640/2;
unsigned int  centerY = 480/2;


/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void){
  term_send_str_crlf(" GT    ......vygeneruje trojuhelnik");
  term_send_str_crlf(" LOAD  ......vygeneruje plochu");
  term_send_str_crlf(" START ......spusti animaci");
  term_send_str_crlf(" STOP  ......zastavi animaci");
}

unsigned int mrand(unsigned int lim)
{
  unsigned int res = rand();
  return res % lim;
}

char SetTriangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, unsigned char clr, unsigned char cls)
{
  unsigned char data[8];

  // Out of range
  if (x1 >= 512) return -1;
  if (y1 >= 480) return -1;
  if (x2 >= 512) return -1;
  if (y2 >= 480) return -1;

  data[7] = x1 & 0xFF;                               // FPGA (7 downto 0)
  data[6] = ((x1 >> 8) & 0x01) + ((y1 << 1) & 0xFF); // FPGA (15 downto 8)
  data[5] = ((y1 >> 7) & 0x03) + ((x2 << 2) & 0xFF); // FPGA (23 downto 16)
  data[4] = ((x2 >> 6) & 0x07) + ((y2 << 3) & 0xFF); // FPGA (31 downto 24)
  data[3] = ((y2 >> 5) & 0x0F) + ((x3 << 4) & 0xFF); // FPGA (39 downto 32)
  data[2] = ((x3 >> 4) & 0x1F) + ((y3 << 5) & 0xFF); // FPGA (47 downto 40)
  data[1] = ((y3 >> 3) & 0x3F) + ((cls & 1) << 7);   // FPGA (55 downto 48)
  data[0] = clr;                                     // FPGA (63 downto 56)

  FPGA_SPI_RW_AN_DN(1, 0, &(data[0]), 1, 8);

  return 0;
}

void VGA_GENPIC(unsigned char id)
{
  int i, a, b;
  char res;

  switch (id)
  {
    case 0x01:
      SetTriangle(centerX, centerY, centerX-50, centerY, centerX, centerY-50, mrand(256), 1);
      SetTriangle(centerX, centerY, centerX+50, centerY, centerX, centerY-50, mrand(256), 0);
      SetTriangle(centerX, centerY, centerX-50, centerY, centerX, centerY+50, mrand(256), 0);
      SetTriangle(centerX, centerY, centerX+50, centerY, centerX, centerY+50, mrand(256), 0);
      break;
    
    case 0x02:
      a = 0;
      b = 10;
      for (i=0; i<20; i++)
      {
        if ((i % 2) == 0)
        {
          SetTriangle(a, 200, b, 250, a+20, 200, mrand(256), 0);
          a += 20;
        }
        else
        {
          SetTriangle(b, 250, a, 200, b+20, 250, mrand(256), 0);
          b += 20;
        }
      }
      break;
      
    case 0x03: // LOAD
      a = 1;
      for (i=0; i<DATALEN; i++)
      {
        SetTriangle(DATA[i][0], DATA[i][1], DATA[i][2], DATA[i][3], DATA[i][4], DATA[i][5], DATA[i][6], a);
        a = 0;
      }
      break;

    case 0xFF:
      res = SetTriangle(mrand(512), mrand(480), mrand(512), mrand(480), mrand(512), mrand(480), mrand(256), 1);
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
  if ((strlen(cmd_ucase) == 2) && (strcmp2(cmd_ucase, "GT")))
    VGA_GENPIC(0xFF);
  else if ((strlen(cmd_ucase) == 5) && (strcmp5(cmd_ucase, "START")))
    Animate = 1;
  else if ((strlen(cmd_ucase) == 4) && (strcmp4(cmd_ucase, "STOP")))
    Animate = 0;
  else if ((strlen(cmd_ucase) == 4) && (strcmp4(cmd_ucase, "LOAD")))
    VGA_GENPIC(0x03);
  else
    return (CMD_UNKNOWN);

  return (USER_COMMAND);
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized(){
  VGA_GENPIC(0x01);
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
    delay_ms(10);  //zpozdeni 1ms
    cnt++;
    if (cnt == 50) 
    {
      flip_led_d6(); //invertovat LED
      if (Animate)
      {
        AnimateCnt += 1;
        VGA_GENPIC(0x02);
      }
      cnt = 0;
    }
    terminal_idle();  // obsluha terminalu
  }         
}
