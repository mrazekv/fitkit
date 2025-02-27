/*******************************************************************************
   ticktacktoe.h: Tick tack toe - functions
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>
              Zdenek Vasicek <vasicek AT fit.vutbr.cz>

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

#include <keyboard/keyboard.h>
#include <stdlib.h>
#include "ticktacktoe.h"


//umisti krizek nebo kolecko do mrizky
void ticktacktoe_place(unsigned char x, unsigned char y, unsigned char id) 
{
  id = (id & 0x1) + 2;
  indexmap_write(x + BOARD_XOFFSET, y + BOARD_YOFFSET, &id);
}

//zjisti, zda je na dane pozici krizek nebo kolecko
char ticktacktoe_check(unsigned char x, unsigned char y) 
{
  unsigned char id;
  indexmap_read(x + BOARD_XOFFSET, y + BOARD_YOFFSET, &id);
  if (id < 2) return -1;
  return id-2;
}

//inicializace hraciho pole
void ticktacktoe_init_board() 
{
  unsigned char id;
  int x, y;

  for (x=0; x < MAXWIDTH; x++)
    for (y=0; y < MAXWIDTH; y++) {
        id = 0;
        if ((x >= BOARD_XOFFSET) && (x < BOARD_XOFFSET+BOARD_WIDTH) &&
            (y >= BOARD_YOFFSET) && (y < BOARD_YOFFSET+BOARD_HEIGHT)) {
           id = 1;
        }
        indexmap_write(x, y, &id);
    }
}

//inicializace textur
void ticktacktoe_init_textures(char gray) 
{
  unsigned char data[2];
  unsigned int  i, j, texture_id, x, y, r;
  unsigned int  color;
  unsigned char border;

  // Vygenerovani textur: 64x64 pixelu
  // rozdeleno na 4 textury o rozmeru 32x32
  for (i=0; i < TEXTURE_WIDTH; i++) 
  {
    for (j=0; j < TEXTURE_WIDTH; j++) 
    {
        // Texture ID 
        texture_id = 2;
        if (i < TEXTURE_PWIDTH) texture_id=0;
        if (j >= TEXTURE_PWIDTH) texture_id++;
      
        x = ((texture_id & 1) == 0) ? j : j - TEXTURE_PWIDTH;
        y = ((texture_id & 2) == 0) ? i : i - TEXTURE_PWIDTH;
 
        //test, jedna-li se o ramecek
        border = (x==0) || (y==0) || (x==(TEXTURE_PWIDTH-1)) || (y==(TEXTURE_PWIDTH-1));

       // RGB barva 3+3+3 bity
       color = 0;
       switch (texture_id) 
       {
          case 0: //pozadi
            color = 0x003;
            break;

          case 1: //prazdny ctverecek
            color = (border) ? 0x0FF : 0x000;
            break;

          case 2: //kolecko
            color = 0x0FF;
            if (!border) 
            {
               r = (x-TEXTURE_QWIDTH+1)*(x-TEXTURE_QWIDTH+1) + (y-TEXTURE_QWIDTH+1)*(y-TEXTURE_QWIDTH+1);
               color = ((r >= 50) && (r < 162)) ? ((gray==0)?0x092:0x00C) : 0x000;
            }
            break;
     
          case 3: //krizek
            color = 0x0FF;
            if (!border) 
            {
               color = ((abs(x - y) <= 3) || (abs((TEXTURE_PWIDTH-x)-y) <= 3)) ?  ((gray==1)?0x092:0x060) : 0x000;
            }
            break;
       }

       data[0] = (unsigned char)((color >> 8) & 255);
       data[1] = (unsigned char)(color & 255);

       texture_write(j,i, &(data[0]));
    }
    WDG_reset();
  }
}

//zmena souradnic kurzoru
void ticktacktoe_cursor_move(unsigned char x, unsigned char y) 
{
  unsigned char data[2];
  data[0] = y + BOARD_YOFFSET;
  data[1] = x + BOARD_XOFFSET;

  cursor_position_write(&(data[0]));
}

//Zjisteni stavu klavesnice
unsigned short oldkeyst = 0;

unsigned short ticktacktoe_kbstate(void) 
{
  unsigned char data[2];
  unsigned short keyst;

  keyboard_read(&(data[0]));
  
  // V poli jsou prijata data z FPGA
  keyst = ((unsigned short)(data[0]) << 8) + data[1];

  if (keyst == oldkeyst) return CMD_NONE;
  oldkeyst = keyst;

  //TODO: CMD pro sikmo nahoru doleva, sikmo nahoru doprava, ...

  if (keyst & KEY_4) return CMD_LEFT;
  if (keyst & KEY_6) return CMD_RIGHT;
  if (keyst & KEY_2) return CMD_UP;
  if (keyst & KEY_8) return CMD_DOWN;
  if (keyst & KEY_5) return CMD_PLACE;

  return CMD_NONE;
}


//struktura pro uchovani delky nejdelsi souvisle sekvence symbolu
typedef struct 
{
  unsigned char l1;
  unsigned char max_l1;
  unsigned char l2;
  unsigned char max_l2;
} LL;

//vypocet delky souvisle sekvence
char check(char x, char y, LL *ll) 
{
  char d1 = 1,d2 = 1,ch;
 
  ch = ticktacktoe_check(x,y);
  
  if (ch == 0) {
     ll->l1++;
     d1 = 0;
  } else if (ch == 1) {
     ll->l2++;
     d2 = 0;
  }

  if (d2) {
     if (ll->l2 > ll->max_l2) ll->max_l2=ll->l2;
     ll->l2=0;
  }   
  if (d1) {
     if (ll->l1 > ll->max_l1) ll->max_l1=ll->l1;
     ll->l1=0;
  }   

  return ch;
}

//Zjisti, zda-li nektery z hracu vyhral
//Vraci: 0 - pokud vyhral hrac cislo 1, 
//       1 - pokud hrac cislo 2, 
//       2 - pokud je remiza (pokryta cela plocha, ale neni vitez)
//      -1 - pokud nevyhral nikdo
int ticktacktoe_check_winner() 
{
  char x,y;
  short sum_area;
  LL ll;

  ll.max_l1=0; ll.max_l2=0; sum_area=0;

  //horizontalne
  for (y=0; y<BOARD_HEIGHT; y++) 
  {
      ll.l1=0; ll.l2=0;
      for (x=0; x<BOARD_WIDTH;x++) 
      { 
          if (check(x,y, &ll) != -1) 
          {
             sum_area++; //spocitat pokrytou plochu
          }
      }
  }

  //vertikalne 
  for (x=0; x<BOARD_WIDTH; x++) 
  {
      ll.l1=0; ll.l2=0;
      for (y=0; y<BOARD_HEIGHT; y++) 
          check(x,y, &ll);
  } 

  //krizem zleva doprava
  for (x=-(BOARD_WIDTH-(SYMBOLS_TO_WIN+1)); x < BOARD_WIDTH-(SYMBOLS_TO_WIN-1); x++) 
  {
      ll.l1=0; ll.l2=0;
      for (y=0; y<BOARD_HEIGHT; y++) 
      {
         if ((x+y < 0) || (x+y >= BOARD_WIDTH)) continue;
         check(x+y,y, &ll);
     }
  } 
 
  //krizem zprava doleva
  for (x=SYMBOLS_TO_WIN-1; x < 2*BOARD_WIDTH-(SYMBOLS_TO_WIN+1); x++) 
  {
      ll.l1=0; ll.l2=0;
      for (y=0; y<BOARD_HEIGHT; y++) 
      {
          if ((x-y < 0) || (x-y >= BOARD_WIDTH)) continue;
          check(x-y,y, &ll);
      }
  } 

  if ((ll.max_l1 >= SYMBOLS_TO_WIN) && (ll.max_l1 > ll.max_l2)) return 0;
  if ((ll.max_l2 >= SYMBOLS_TO_WIN) && (ll.max_l2 > ll.max_l1)) return 1;
  if (sum_area == BOARD_WIDTH*BOARD_HEIGHT) return 2;

  return -1;
}

