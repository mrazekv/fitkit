/*******************************************************************************
   main.c: Life game
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
 
   This software is provided ``as is'', and any express or implied
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
#include <thermometer/thermometer.h>
#include <lcd/display.h>
#include <stdlib.h> //rand, srand
#include "vga_block.h"
#include "lifectrl.h"

#define CELL_DEAD 0
#define CELL_COMETOLIFE 2
#define CELL_LIVE 3

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help") 
*******************************************************************************/
void print_user_help(void)
{
    term_send_str_crlf(" GEN - vygeneruje nahodne populaci");
    term_send_str_crlf(" ENA - povoli cinnost automatu");
    term_send_str_crlf(" DIS - zakaze cinnost automatu");
}

unsigned short rseed;

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) 
{
  unsigned short x, y;

  if (strcmp3(cmd_ucase,"GEN")) 
  {
     for (y=0; y < VGA_ROWS; y++)
        for (x=0; x < VGA_COLUMNS; x++) 
        {
            rseed = rand();
            if ((rseed & 0xFF) > 128) 
            {
               if (x + y < VGA_COLUMNS) 
               {
                  VGA_SetPixelXY(x, y, CELL_COMETOLIFE);
               } 
               else 
               {
                  VGA_SetPixelXY(x, y, CELL_LIVE);
               }
            } 
            else 
            {
               VGA_SetPixelXY(x, y, CELL_DEAD);
            }
        }
  } 
  else if (strcmp3(cmd_ucase,"ENA")) 
  {
     life_enable();

  } 
  else if (strcmp3(cmd_ucase,"DIS")) 
  {
     life_disable();

  }
  else 
  {
    return (CMD_UNKNOWN);
  }
  return USER_COMMAND;
}

//Vykresli glider (posunujici se objekt)
void glider(unsigned short x, unsigned short y) 
{
  VGA_SetPixelXY(x+1, y+0, CELL_LIVE);  
  VGA_SetPixelXY(x+2, y+1, CELL_LIVE);
  VGA_SetPixelXY(x+0, y+2, CELL_LIVE);
  VGA_SetPixelXY(x+1, y+2, CELL_LIVE);  
  VGA_SetPixelXY(x+2, y+2, CELL_LIVE);
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized(void)
{
  rseed = (thermometer_measure() & 0xFFFF);
  srand(rseed);
  term_send_str("Seed:");
  term_send_num(rseed);
  term_send_crlf();
      
  LCD_init();
  LCD_write_string("Life...");

  VGA_Clear();

  glider(10,10);
  glider(30,30);
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void) 
{
  initialize_hardware();
 
  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5  
 
  WDG_stop();                          // zastav watchdog

  while (1)  
  {
    terminal_idle();                    // obsluha terminalu
  }  
} 
