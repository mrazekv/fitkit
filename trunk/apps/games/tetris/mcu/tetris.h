//IMP project, Izidor Matusov, xmatus19, my original file
//
/*******************************************************************************
   tetris.h: Functions && macros for work with game structures
   Copyright (C) 2009, 2010 Brno University of Technology,
                            Faculty of Information Technology
   Author(s): Izidor Matusov <xmatus19 AT stud.fit.vutbr.cz>

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

#ifndef __TETRIS_H__
#define __TETRIS_H__

#include <stdbool.h>

#include "types.h"

/**
 * Dimmensions of the board
 */
#define DIM_HEIGHT 20
#define DIM_WIDTH 10

/**
 * Maximal allowed level - 15 is pretty tough!
 */
#define LEVEL_MAX 15+1

/**
 * Structure for speed constants
 */
typedef struct {
    unsigned pointsToNextSpeed;
    unsigned delay;
} speed_t;

#define SPEEDS_COUNT 10
extern const speed_t speeds[SPEEDS_COUNT];

/**
 * Maximal dimmensions of a block
 */
#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH  4

/**
 * Functions protypes
 */

block_t randomBlock(void);
block_t rotateBlock(const block_t block);
void generateBoard(const unsigned level);
void printBlock(const coord_t x, const coord_t y, const block_t block, const color_t color, const bool boardCoord);
void eraseBlock(const coord_t x, const coord_t y, const block_t block, const bool boardCoord);
bool isPossiblePlace(const coord_t x, const coord_t y, const block_t block);
void placeBlock(const coord_t x, const coord_t y, const block_t block);
unsigned short removeFullLines(void);
#endif
