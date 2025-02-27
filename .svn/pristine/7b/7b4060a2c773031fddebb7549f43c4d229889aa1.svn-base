//IMP project, Izidor Matusov, xmatus19, my original file
//
/*******************************************************************************
   tetris.c: Functions && macros for work with game structures
   Copyright (C) 2009, 2010 Brno University of Technology,
                            Faculty of Information Technology
   Author(s): Izidor Matusov <xmatus19 AT stud.fit.vutbr.cz>
              Michal Bidlo   <bidlom AT fit.vutbr.cz>

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

#include <stdlib.h>
#include <stdbool.h>

#include <fitkitlib.h>

#include "tetris.h"
#include "types.h"
#include "utils.h"

/**
 * Speed constatns
 */
const speed_t speeds[SPEEDS_COUNT] = {
    {   5, 0x5000 }, //speed 0
    {  10, 0x4700 }, //speed 1
    {  20, 0x4000 }, //speed 2
    {  30, 0x3700 }, //speed 3
    {  40, 0x3000 }, //speed 4
    {  50, 0x2300 }, //speed 5
    {  70, 0x1F00 }, //speed 6
    {  90, 0x1400 }, //speed 7
    { 100, 0x1200 }, //speed 8
    { 999, 0x0E00 }, //speed 9 - maximum speed
};

/**
 * Static variable for board state
 */
static boardline_t board[DIM_HEIGHT] = {0};
#define FULL_LINE 0x3FF

/**
 * Structure for block
 */
typedef struct {
    unsigned char shape[2]; //hexadecimal coding of bitmap mask
    unsigned next_block; //rotate will change index of block to this value
} block_definition_t;

/**
 * Definitions of blocks
 *
 * Each time there is bitmap mask of a block, 
 * which is coded into hexadecimal code. It's comfortable 
 * for adding new blocks and quite good when working with it.
 *
 * Prepare for bitwise operations ]:->
 */
#define BLOCKS_COUNT 19
const block_definition_t blocks[BLOCKS_COUNT] = {
   /*
    * 0100 4
    * 1100 C 
    * 1000 8
    * 0000 0
    */
   { {0x4C, 0x80}, 1 }, // 0
   /*
    * 1100 C
    * 0110 6
    * 0000 0 
    * 0000 0
    */
   { {0xC6, 0x00}, 0 }, // 1
   /*
    * 1000 8
    * 1100 C
    * 0100 4
    * 0000 0
    */
   { {0x8C, 0x40}, 3 }, // 2
   /*
    * 0110 6
    * 1100 C
    * 0000 0
    * 0000 0
    */
   { {0x6C, 0x00}, 2 }, // 3
   /*
    * 1100 C
    * 1100 C
    * 0000 0
    * 0000 0
    */
   { {0xCC, 0x00}, 4 }, // 4
   /*
    * 1000 8
    * 1000 8
    * 1000 8
    * 1000 8
    */
   { {0x88, 0x88}, 6 }, // 5
   /*
    * 1111 F
    * 0000 0
    * 0000 0
    * 0000 0
    */
   { {0xF0, 0x00}, 5 }, // 6
   /*
    * 1000 8
    * 1100 C
    * 1000 8
    * 0000 0
    */
   { {0x8C, 0x80}, 10 }, // 7
   /*
    * 1110 E
    * 0100 4
    * 0000 0 
    * 0000 0
    */
   { {0xE4, 0x00}, 7 }, // 8
   /*
    * 0100 4
    * 1100 C
    * 0100 4
    * 0000 0
    */
   { {0x4C, 0x40}, 8 }, // 9
   /*
    * 0100 4
    * 1110 E
    * 0000 0 
    * 0000 0
    */
   { {0x4E, 0x00}, 9 }, // 10
   /*
    * 1000 8
    * 1000 8
    * 1100 C
    * 0000 0
    */
   { {0x88, 0xC0}, 14 }, // 11
   /*
    * 1110 E
    * 1000 8
    * 0000 0 
    * 0000 0
    */
   { {0xE8, 0x00}, 11 }, // 12
   /*
    * 1100 C
    * 0100 4
    * 0100 4
    * 0000 0
    */
   { {0xC4, 0x40}, 12 }, // 13
   /*
    * 0010 2
    * 1110 E
    * 0000 0 
    * 0000 0
    */
   { {0x2E, 0x00}, 13 }, // 14

   /*
    * 0100 4
    * 0100 4
    * 1100 C
    * 0000 0
    */
   { {0x44, 0xC0}, 18 }, // 15
   /*
    * 1000 8
    * 1110 E
    * 0000 0 
    * 0000 0
    */
   { {0x8E, 0x00}, 15 }, // 16
   /*
    * 1100 C
    * 1000 8
    * 1000 8
    * 0000 0
    */
   { {0xC8, 0x80}, 16 }, // 17
   /*
    * 1110 E
    * 0010 2
    * 0000 0 
    * 0000 0
    */
   { {0xE2, 0x00}, 17 }, // 18
};

/**
 * Generate random block
 *
 * @return  block number
 */
block_t randomBlock(void) {
    return ((unsigned) rand() % 0x00FF) % BLOCKS_COUNT;
}

/**
 * Rotate block
 *
 * It may be hard to rotate properly but there is a small hack. 
 * You remember the next block which the block will rotate.
 * Then you just change index number.
 *
 * @param   block to rotate
 * @return  new block
 */
block_t rotateBlock(const block_t block) {
    return blocks[block].next_block;
}

/**
 * Generate board for the given level
 *
 * @param   level - how many lines should be generated
 */
void generateBoard(const unsigned level) {
    coord_t i;
    for (i = 0; i < level; i++) {
        unsigned short index = DIM_HEIGHT-1-i;
        do {
            board[index]  = (unsigned) rand() % FULL_LINE;
        } while (board[index] == FULL_LINE || board[index] == 0);

        VGA_DrawBoardLine(index, board[index]);
    }

    //other lines should be blank
    for (; i < DIM_HEIGHT; i++) {
        unsigned short index = DIM_HEIGHT-1-i;
        board[index] = 0;

        VGA_DrawBoardLine(index, board[index]);
    }
}

/**
 * Print block
 *
 * It use some bitwise operation :-)
 *
 * @param   x - left x-position of the block/real
 * @param   y - top y-position of the block/real
 * @param   block - index of block to be printed
 * @param   color - which color to print
 * @param   boardCoord - are the coordination on board? (if not they are real)
 */
void printBlock(const coord_t x, const coord_t y, const block_t block, const color_t color, const bool boardCoord) {
    coord_t by, bx;

    //skip unknown blocks
    if (block >= BLOCKS_COUNT) {
        return;
    }

    for (by = 0; by < BLOCK_HEIGHT; by++) {
        for (bx = 0; bx < BLOCK_WIDTH; bx++) {
            unsigned short index = by < 2 ? 0 : 1;
            unsigned short bit_index = ( by == 0 || by == 2 ) ? BLOCK_WIDTH : 0;
            bit_index += BLOCK_WIDTH-bx-1;

            if ( (blocks[block].shape[index] >> bit_index) & 1 ) {
                if (boardCoord) {
                    VGA_SetBoardPointXY(x+bx, y+by, color);
                }
                else {
                    coord_t base_x = x + 2*bx;
                    coord_t base_y = y + 2*by;

                    VGA_SetPixelXY(base_x,   base_y,   color);
                    VGA_SetPixelXY(base_x+1, base_y,   color);
                    VGA_SetPixelXY(base_x,   base_y+1, color);
                    VGA_SetPixelXY(base_x+1, base_y+1, color);
                }
            }
        }
    }
}

/**
 * Erase block
 *
 * Macro for printing block with black color
 *
 * @param   x - left x-position of the block
 * @param   y - top y-position of the block
 * @param   block - index of block to be printed
 * @param   boardCoord - are the coordination on board? (if not they are real)
 */
void eraseBlock(const coord_t x, const coord_t y, const block_t block, const bool boardCoord) {
            printBlock(x,y, block, VGA_BLACK, boardCoord);
}

/**
 * Is it possible to place at these coordination?
 *
 * Checks if:
 *  - every piece of block is on board
 *  - every piece is on unallocated piece of board
 *
 * @param   x - coordination on board
 * @param   y - coordination on board
 * @param   block - block type
 * @return  is possible for block to be on the position?
 */
bool isPossiblePlace(const coord_t x, const coord_t y, const block_t block) {
    coord_t by, bx;

    for (by = 0; by < BLOCK_HEIGHT; by++) {
        for (bx = 0; bx < BLOCK_WIDTH; bx++) {
            unsigned short index = by < 2 ? 0 : 1;
            unsigned short bit_index = ( by == 0 || by == 2 ) ? BLOCK_WIDTH : 0;
            bit_index += BLOCK_WIDTH-bx-1;

            if ( (blocks[block].shape[index] >> bit_index) & 1) { 
                if (y + by >= DIM_HEIGHT  || x + bx >= DIM_WIDTH || board[y+by] >> (DIM_WIDTH -1- (x+bx)) & 1) {
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * Place the block on these coordination (but do no check) and print it
 *
 * @param   x - coordination on board
 * @param   y - coordination on board
 * @param   block - block type
 */
void placeBlock(const coord_t x, const coord_t y, const block_t block) {
    coord_t by, bx;

    for (by = 0; by < BLOCK_HEIGHT; by++) {
        for (bx = 0; bx < BLOCK_WIDTH; bx++) {
            unsigned short index = by < 2 ? 0 : 1;
            unsigned short bit_index = ( by == 0 || by == 2 ) ? BLOCK_WIDTH : 0;
            bit_index += BLOCK_WIDTH-bx-1;

            if ( (blocks[block].shape[index] >> bit_index) & 1) { 
                board[y+by] |= 1 << (DIM_WIDTH-1-(x+bx));
                VGA_SetBoardPointXY(x+bx, y+by, VGA_FALLEN);
            }
        }
    }
}

/**
 * Remove full lines from board and return count of points gain
 *
 * @return  how many points player has get
 */
unsigned short removeFullLines(void) {
    //Warning: these variables can`t be unsigned!!
    int y;
    int fallTo = DIM_HEIGHT-1;

    unsigned short points = 0;

    for (y = DIM_HEIGHT-1; y >= 0; y--) {
        if (board[y] == FULL_LINE) {
            points += 1;

            /* Animation for full line */
            unsigned short i;
            for (i = 0; i <= DIM_WIDTH; i++) {
                VGA_DrawBoardLine(y, board[y]);
                board[y] >>= 1;

                if (i+1 < DIM_WIDTH) {
                    VGA_SetBoardPointXY(i+1, y, randomColor());
                }

                delay_ms(50);
            }
        }
        else {
            board[fallTo] = board[y];
            VGA_DrawBoardLine(fallTo, board[fallTo]);
            //Erase current line to erase previous blocks
            if (fallTo != y) {
                VGA_DrawBoardLine(y, 0);
            }
            fallTo--;
        }
    }

    //zero other lines
    for (; fallTo >= 0; fallTo--) {
        board[fallTo] = 0;
        VGA_DrawBoardLine(fallTo, board[fallTo]);
    }

    //Disable falling immediatelly after removing lines
    if (points > 0) {
        extern bool fall_flag;
        fall_flag = false;
    }

    return points;
}
