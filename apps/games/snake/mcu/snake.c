/*******************************************************************************
   snake.c: Snake functions
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Karel Slany    <slany AT fit.vutbr.cz>

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
#include <stdlib.h>
#include "vga_block.h"
#include "snake.h"

/*******************************************************************************
 * Funkce ovladani hada
 *******************************************************************************/

  // (0x0DD0 0000 0000 0000 >> 13) & 0x03 == smer
  // (0x000X XXXX XX00 0000 >> 6) & 0x7f  == x-sloupec
  // (0x0000 0000 00YY YYYY) & 0x3f       == y-radek

// prida na zacatek hada prvek
void snake_add_head(unsigned char x, unsigned char y, unsigned char d) {
  
  snake_t new_snake_head = SNAKE_DXY(d, x, y);
 
  if (snake_struct_length == 0) {
    // pokud je to prvni prvek do struktury
      // vloz 2x
    DEC_SNAKE_I(snake_head_i);
    snake_struct_length++;
    snake_tail_i = snake_head_i; // nastavi se konec ocasu
    snake[snake_head_i] = new_snake_head;

    DEC_SNAKE_I(snake_head_i);
    snake_struct_length++;
    snake[snake_head_i] = new_snake_head;
    return;
  }

  if (d == GET_D(snake[snake_head_i])) {
    // pokud je zachovan stejny smer
      // zmen jen souradnice
    snake[snake_head_i] = new_snake_head;
  } else {
    if (snake_struct_length == MAX_SNAKE_STRUCT_LENGTH) {
      // maximalni delka struktury hada dosazena
      // zakaze preruseni casovace
      //CCTL0 = 0;
      return;
    }
    // pokud je jiny smer
      // vloz souradnice a smer
      // zvetsi delku o jedna
    DEC_SNAKE_I(snake_head_i);
    snake_struct_length++;
    snake[snake_head_i] = new_snake_head;
  }
}

// nacte souradnice konce hada
void snake_get_tail(unsigned char *x, unsigned char *y, unsigned char *d) {

  if (snake_struct_length) {
    *y = GET_Y(snake[snake_tail_i]);
    *x = GET_X(snake[snake_tail_i]);
    *d = GET_D(snake[snake_tail_i]);
  }

}

// smaze konec hada
void snake_cut_tail() {

  if (snake_struct_length) {

    unsigned char ax, ay;
    unsigned char prevx, prevy, prevd;
    unsigned int  prev_tail_i = snake_tail_i;
    DEC_SNAKE_I(prev_tail_i);

    // vypocita souradnice ocasu, pokud by se nachazel ve 4-okoli
    prevx = GET_X(snake[prev_tail_i]);
    prevy = GET_Y(snake[prev_tail_i]);
    prevd = GET_D(snake[prev_tail_i]);

    switch (prevd) {
      case D_UP :
        ax = prevx;
        ay = Y_DOWN(prevy);
        break;
      case D_RIGHT :
        ax = X_LEFT(prevx);
        ay = prevy;
        break; 
      case D_DOWN :
        ax = prevx;
        ay = Y_UP(prevy);
        break;
      case D_LEFT :
        ax = X_RIGHT(prevx);
        ay = prevy;
        break; 
      default :
        break;
    }

    if ((GET_X(snake[snake_tail_i]) == ax) &&
        (GET_Y(snake[snake_tail_i]) == ay)) {
        // pokud se prvek nachazi v jeho 4-okoli
        // smaz ocasni prvek, zmen delku struktury, zmen ukazatel
        // a zmen smer
        snake_tail_i = prev_tail_i;
        snake_struct_length--;
        if (snake_struct_length == 1) {
          // pokud je delka 1
            // pak smaz i zacatek, nastav velikost struktury na 0
          snake_tail_i = snake_head_i;
          snake_struct_length--;
        } else {
          // tohle je to "a zmen smer"
          DEC_SNAKE_I(prev_tail_i);
          SET_D(snake[snake_tail_i], GET_D(snake[prev_tail_i]));
        }
    } else {
      // pokud prvek ve strukture hada pred ocasem neni v jeho 4-okoli
        // prepis souradnice ocasu, vypocitej je z predocasniho prvku
      switch (GET_D(snake[snake_tail_i])) {
        case D_UP :
          SET_Y(snake[snake_tail_i], Y_UP(GET_Y(snake[snake_tail_i])));
          break;

        case D_RIGHT :
          SET_X(snake[snake_tail_i], X_RIGHT(GET_X(snake[snake_tail_i])));
          break;

        case D_DOWN :
          SET_Y(snake[snake_tail_i], Y_DOWN(GET_Y(snake[snake_tail_i])));
          break;
        
        case D_LEFT :
          SET_X(snake[snake_tail_i], X_LEFT(GET_X(snake[snake_tail_i])));
          break;
        default :
          break;
      }
    }
  }
}

// porovna souradnici s telem hada
unsigned short is_snake_body(unsigned char x, unsigned char y) {

  unsigned int a_index = snake_head_i;
  unsigned int b_index = snake_head_i;

  unsigned char ax, ay;

  INC_SNAKE_I(b_index);

  unsigned short found = 0;
  
  if (snake_struct_length) {
    // vzdycky jsou zde aspon 2 prvky, hlavicka a ocasek

    while ((!found) && (a_index != snake_tail_i)) {

      switch (GET_D(snake[a_index])) {
         case D_UP :
           ax = GET_X(snake[a_index]);
           ay = GET_Y(snake[a_index]);
           while ((!found) && (ay != GET_Y(snake[b_index]))) {
             found = ((ax == x) && (ay == y));
             ay = Y_DOWN(ay);
           }
           break;

         case D_RIGHT :
           ax = GET_X(snake[a_index]);
           ay = GET_Y(snake[a_index]);
           while ((!found) && (ax != GET_X(snake[b_index]))) {
             found = ((ax == x) && (ay == y));
             ax = X_LEFT(ax);
           }
           break;

         case D_DOWN :
           ax = GET_X(snake[a_index]);
           ay = GET_Y(snake[a_index]);
           while ((!found) && (ay != GET_Y(snake[b_index]))) {
             found = ((ax == x) && (ay == y));
             ay = Y_UP(ay);
           }
           break;
         case D_LEFT :
           ax = GET_X(snake[a_index]);
           ay = GET_Y(snake[a_index]);
           while ((!found) && (ax != GET_X(snake[b_index]))) {
             found = ((ax == x) && (ay == y));
             ax = X_RIGHT(ax);
           }
           break;

         default :
           break;
      }

      INC_SNAKE_I(a_index);
      INC_SNAKE_I(b_index);

    }

    return found;
  } else {
    return 0;
  }
}

// vygeneruje souradnice mnamky
void gen_yummy(unsigned char *x, unsigned char *y) {

   unsigned char ax, ay;

  do {
    ax = (((unsigned) rand() & 0x00ff) % ((VGA_COLUMNS) - 2)) + 1;
    ay = (((unsigned) rand() & 0x00ff) % ((VGA_ROWS) - 2)) + 1;
  } while (is_snake_body(ax, ay));

  *x = yummy_x = ax;
  *y = yummy_y = ay;

}

// vrati 1, pokud bylo neco ke snedku
unsigned short is_yummy(unsigned char x, unsigned char y) {

  return (x == yummy_x) && (y == yummy_y);

}
