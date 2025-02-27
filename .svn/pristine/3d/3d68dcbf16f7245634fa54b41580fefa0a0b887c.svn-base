//IMP project, Izidor Matusov, xmatus19, inspired by keyboard and vga_block modules from snake/, 
//my work cca 90%, used library for writing into VideoRAM
//
/*******************************************************************************
   utils.h: Functions && macros for working with pheripherals (LCD, VGA, Keyboard)
   Copyright (C) 2009, 2010 Brno University of Technology,
                            Faculty of Information Technology
   Author(s): Izidor Matusov <xmatus19 AT stud.fit.vutbr.cz>
              Zdenek Vasicek <vasicek AT fit.vutbr.cz>
              Karel Slany    <slany AT fit.vutbr.cz>

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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "types.h"

/**
 * Dimensions of VideoRAM
 */
#define VGA_COLUMNS 64
#define VGA_ROWS    60

/**
 * Codes for colors
 */
#define VGA_BLACK 0
#define VGA_WALL 1
#define VGA_FALLEN 2

#define VGA_RAND01  3
#define VGA_RAND02  4
#define VGA_RAND03  5
#define VGA_RAND04  6
#define VGA_RAND05  7
#define VGA_RAND06  8
#define VGA_RAND07  9
#define VGA_RAND08 10
#define VGA_RAND09 11
#define VGA_RAND10 12
#define VGA_RAND11 13
#define VGA_RAND12 14
#define VGA_RAND13 15

/**
 * Random colors for new blocks
 */
#define VGA_RAND_START 3
#define VGA_RAND_COUNT 13

/**
 * Start position of screen
 */
#define SCREEN_COL_START 16
#define SCREEN_ROW_START 9
/**
 * Macros for board definition
 */
#define VGA_COL_START SCREEN_COL_START+1
#define VGA_COL_END VGA_COL_START+2*DIM_WIDTH+1

#define VGA_ROW_START SCREEN_ROW_START+0
#define VGA_ROW_END VGA_ROW_START + 2*DIM_HEIGHT + 1

/**
 * Where the speed drawing starts
 */
#define VGA_SPEED_COL_START SCREEN_COL_START+26
#define VGA_SPEED_ROW_START SCREEN_ROW_START+23

/**
 * Where next block starts
 */
#define VGA_NEXT_COL_START SCREEN_COL_START+28
#define VGA_NEXT_ROW_START SCREEN_ROW_START+6

void VGA_Clear(void);
void VGA_SetPixelXY(const coord_t x, const coord_t y, const color_t color);
void VGA_SetBoardPointXY(const coord_t x, const coord_t y, const color_t color);
void VGA_DrawBoardLine(const coord_t y, const boardline_t boardline);
void VGA_DrawScene(void);
void printGameOver(void);
void printSpeed(const unsigned short speed);
void printNextBlock(const block_t block, const color_t color);
color_t randomColor(void);

/**
 * Keyboard actions
 */
#define ACT_NONE  0x0
#define ACT_DOWN  0x1 
#define ACT_LEFT  0x2 
#define ACT_RIGHT 0x4
#define ACT_ROTATE 0x8

unsigned char kbstate(void);

/**
 * Write point status to LCD
 */
void LCD_WritePoints(const unsigned points);

#endif
