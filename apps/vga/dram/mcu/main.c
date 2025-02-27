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

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" GENPIC......vygeneruje obrazek");
}

inline int iabs(int v)
{
  if (v < 0)
    return -v;
  else
    return v;
}

void SetPixel(unsigned int x, unsigned int y, unsigned char clr)
{
  // Out of range
  if (x >= 512)
    return;
  if (y >= 480)
    return;

  long addr = ((long)y * 512) + (long)x;

  FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, addr, &clr, 4, 1);
}

void ClearScreen()
{
  unsigned int x, y;
  for (y=0; y<480; y++)
    for (x=0; x<512; x++)
      SetPixel(x, y, 0);
}

void Line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char clr)
{
  // Out of range
  if (x1 >= 512)
    return;
  if (y1 >= 480)
    return;
  if (x2 >= 512)
    return;
  if (y2 >= 480)
    return;
  
  int dx = iabs(x2 - x1);
  int dy = iabs(y2 - y1);
  char steep = dy > dx;
  unsigned int t;
 
  if (steep)
  {
    t = x1; x1 = y1; y1 = t;
    t = x2; x2 = y2; y2 = t;
  }
  
  if (x1 > x2)
  {
    t = x1; x1 = x2; x2 = t;
    t = y1; y1 = y2; y2 = t;   
  }

  dx = x2-x1;
  dy = iabs(y2 - y1);
  
  int e = 0;
  int ystep;
  int y = y1;
  int x;
  if (y1 < y2)
    ystep = 1;
  else
    ystep = -1;
  for (x = x1; x <= x2; x++)
  {
    if (steep)
      SetPixel(y, x, clr);
    else
      SetPixel(x, y, clr);
    e = e + dy;
    if (2 * e >= dx)
    {
      y = y + ystep;
      e = e - dx;
    }
  }
}

void Rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char clr)
{
  Line(x,     y,     x + w, y,     clr);
  Line(x,     y,     x,     y + h, clr);
  Line(x + w, y,     x + w, y + h, clr);
  Line(x,     y + h, x + w, y + h, clr);
}

void VGA_GENPIC(void)
{
  unsigned int i;
  
  ClearScreen(); // VERY SLOW!
  for (i=0; i<10; i++)
    Rectangle(20+i*5, 20+i*5, 50+i*8, 50+i*8, ((i+1)*20) % 256);
  for (i=0; i<256; i++)
    Line(128+i, 250, 128+i, 400, i);
  for (i=0; i<10; i++)
    Rectangle(450-i*13, 20+i*5, 50+i*8, 50+i*8, ((i+1)*20) % 256);
}

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp(cmd_ucase, "GENPIC") == 0)
  {
    VGA_GENPIC();
    return USER_COMMAND;
  }
  else if (strcmp3(cmd_ucase, "PX "))
  {
    int ix[2] = {0, 0};
    int i;
     
    char id = 0;
    
    for (i = 3; (i < 16) && (cmd_ucase[i] != 0); i++)
    {
      if ((cmd_ucase[i] >= '0') && (cmd_ucase[i] <= '9'))
      {
        if (id == 0) id = 1;
        ix[id - 1] = (ix[id - 1] * 10) + (cmd_ucase[i] - '0');
      }
      else if (id == 1) id = 2;
      else i = 99;
    }
    
    term_send_str("Put: ");
    term_send_num(ix[0]);
    term_send_str(", ");
    term_send_num(ix[1]);
    term_send_crlf();
    
    SetPixel(ix[0], ix[1], 255);
    
    return USER_COMMAND;
  }
  else
    return CMD_UNKNOWN;
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

  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5

  while (1) {
    delay_ms(10);
    cnt++;
    if (cnt > 50) {
      cnt = 0;
      flip_led_d6();                   // negace portu na ktere je LED
    }
    terminal_idle();                   // obsluha terminalu
  }
}           // MAIN
