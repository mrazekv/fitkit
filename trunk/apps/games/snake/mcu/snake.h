/*******************************************************************************
   snake.h: Snake functions
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

#ifndef _SNAKE_H_
#define _SNAKE_H_

// had
#define INIT_SNAKE_LENGTH 15              // pocatecni delka hada
#define MAX_SNAKE_STRUCT_LENGTH 400              // maximalni delka struktury hada
#define INC_SNAKE_I(index) ((index == (MAX_SNAKE_STRUCT_LENGTH - 1)) ? (index = 0) : (index++))
#define DEC_SNAKE_I(index) ((index == 0) ? (index = (MAX_SNAKE_STRUCT_LENGTH - 1)) : (index--))
typedef unsigned int snake_t;
snake_t *snake;
unsigned int snake_struct_length;     // pocitadlo velikosti struktury hada
unsigned int snake_head_i;            // kde ja zacatek
unsigned int snake_tail_i;            // kde je konec
#define SNAKE_EXTEND_STEP 5               // o kolik se prodlouzi had pri papani
unsigned int snake_extend;            // o kolik bude jeste prodlouzeny had

// zakodovani smeru
#define D_UP 0
#define D_RIGHT 1
#define D_DOWN 2
#define D_LEFT 3

// pocatecni souradnice hada a smer
#define BEGIN_X 40
#define BEGIN_Y 30
#define BEGIN_D D_RIGHT

// souradnice leveho a praveho sloupce hraci plochy
#define MIN_X 1
#define MAX_X VGA_COLUMNS-2
#define X_LEFT(x)  (((x) != MIN_X) ? ((x) - 1) : MAX_X)
#define X_RIGHT(x) (((x) != MAX_X) ? ((x) + 1) : MIN_X)

// souradnice hodniho a spodniho radku plochy
#define MIN_Y 1
#define MAX_Y VGA_ROWS-2
#define Y_UP(y)   (((y) != MIN_Y) ? ((y) - 1) : MAX_Y)
#define Y_DOWN(y) (((y) != MAX_Y) ? ((y) + 1) : MIN_Y)

// maskovani
#define D_MASK (0x03 << 13)
#define X_MASK (0x7f << 6)
#define Y_MASK (0x3f)
#define GET_D(snake_item) (((snake_item) >> 13) & 0x03)
#define GET_X(snake_item) (((snake_item) >> 6) & 0x7f)
#define GET_Y(snake_item) ((snake_item) & 0x3f)
#define SET_D(snake_item, d) ((snake_item) = (((snake_item) & ~D_MASK) | ((0x03 & (snake_t)(d)) << 13)))
#define SET_X(snake_item, x) ((snake_item) = (((snake_item) & ~X_MASK) | ((0x7f & (snake_t)(x)) << 6)))
#define SET_Y(snake_item, y) ((snake_item) = (((snake_item) & ~Y_MASK) | (0x3f & (snake_t)(y))))
#define SNAKE_DXY(d, x, y) ((((((snake_t)((d) & 0x03)) << 7) | (snake_t)((x) & 0x7f)) << 6) | (snake_t)((y) & 0x3f))
// ulozeni souradnic a smeru hada na unsigned int
// (0x0DD0 0000 0000 0000 >> 13) & 0x03 == smer
// (0x000X XXXX XX00 0000 >> 6) & 0x7f  == x-sloupec
// (0x0000 0000 00YY YYYY) & 0x3f       == y-radek

// prida na zacatek hada prvek
void snake_add_head(unsigned char x, unsigned char y, unsigned char d);
// nacte souradnice konce hada
void snake_get_tail(unsigned char *x, unsigned char *y, unsigned char *d);
// smaze konec hada
void snake_cut_tail();
// porovna souradnici s telem hada
unsigned short is_snake_body(unsigned char x, unsigned char y);

unsigned char yummy_x, yummy_y;         // souradnice mnamky

// vygeneruje souradnice papani
void gen_yummy(unsigned char *x, unsigned char *y);
// test na vyskyt papani na souradnicich
unsigned short is_yummy(unsigned char x, unsigned char y);

#endif  // _SNAKE_H_

