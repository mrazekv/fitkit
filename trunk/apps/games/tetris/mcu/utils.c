//IMP project, Izidor Matusov, xmatus19, inspired by keyboard and vga_block modules from snake/, 
//my work cca 90%, used library for writing into VideoRAM
//
/*******************************************************************************
   utils.c: Functions && macros for working with pheripherals (LCD, VGA, Keyboard)
   Copyright (C) 2009, 2010 Brno University of Technology,
                            Faculty of Information Technology
   Author(s): Izidor Matusov <xmatus19 AT stud.fit.vutbr.cz>
              Zdenek Vasicek <vasicek AT fit.vutbr.cz>
              Karel Slany    <slany AT fit.vutbr.cz>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>

#include "utils.h"
#include "tetris.h"
#include "types.h"

/**
 * Local macros
 */
// Base address for VideoRAM
#define BASE_ADDR_VRAM 0x8000
// How many blocks on a single line
#define VGA_STRIDE  VGA_COLUMNS

// abychom zde umoznili i start hry klavesou bez pripojeneho terminalu...
extern bool startNewGame;
// ... a abychom mohli klavesami 0-9 pri spousteni hry zadat level (radky)
extern unsigned short levelInNewGame;

/**
 * Clears the whole VideoRAM
 *
 * Writes everywhere BLACK color
 */
void VGA_Clear(void) {
  unsigned short i, j;
  color_t color = VGA_BLACK;

  for (i=0; i<VGA_ROWS; i++) {
     for (j = 0; j<VGA_COLUMNS; j++) {
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_VRAM/*base*/ + i*VGA_STRIDE + j, &color, 2, 1);
     }
  }
}

/**
 * Change value in the VideoRAM at certain point
 *
 * @param   x - real x-coordination in VideoRAM
 * @param   y - real y-coordination in VideoRAM
 * @param   color - code of color
 */
void VGA_SetPixelXY(const coord_t x, const coord_t y, color_t color) {
  //skip bad coordinations
  if (x >= VGA_COLUMNS || y >= VGA_ROWS) {
      return;
  }

  unsigned short vga_pos = y*VGA_STRIDE + x;
  FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_VRAM /*base*/ + vga_pos, &color, 2, 1);
}

/**
 * Print point on board
 *
 * Calculate real coordinations of point and print it
 *
 * @param   x - x-coordination on the board
 * @param   y - y-coordination on the board
 * @param   color - code of color
 */
void VGA_SetBoardPointXY(const coord_t x, const coord_t y, const color_t color) {
    coord_t bx = VGA_COL_START+1+2*x;
    coord_t by = VGA_ROW_START+1+2*y;

    VGA_SetPixelXY(bx,   by,   color);
    VGA_SetPixelXY(bx+1, by,   color);
    VGA_SetPixelXY(bx,   by+1, color);
    VGA_SetPixelXY(bx+1, by+1, color);
}

/**
 * Redraw a line on board
 *
 * @param   y - y-coordination on the board
 * @param   boardline
 */
void VGA_DrawBoardLine(const coord_t y, const boardline_t boardline) {
    coord_t x;

    for (x = 0; x < DIM_WIDTH; x++) {
        color_t color = boardline >> (DIM_WIDTH-1-x) & 1 ? VGA_FALLEN : VGA_BLACK;
        VGA_SetBoardPointXY(x, y, color);
    }
}

/**
 * Choose random color for new block
 *
 * @return  color code
 */
color_t randomColor(void) {
    return ( ((unsigned) rand() % 0x00FF) % VGA_RAND_COUNT) + VGA_RAND_START;
}

/**
 * Draw initial scene
 */
void VGA_DrawScene(void) {
  short i;
  VGA_Clear();

  color_t color = VGA_WALL;

  for (i=VGA_COL_START;i<VGA_COL_END+1;i++) {
      VGA_SetPixelXY(i,VGA_ROW_END, color);
  }

  for (i=VGA_ROW_START+1;i<VGA_ROW_END;i++) {
      VGA_SetPixelXY(VGA_COL_START,i, color);
      VGA_SetPixelXY(VGA_COL_END,i, color);
  }

  printSpeed(0);
  printNextBlock(0xFF, VGA_BLACK);
}

/**
 * Print Game Over drawing over the board
 *
 */
void printGameOver(void) {
/* Firstly we need to define matrix to draw.
 * To preserve compact format, some macros are used.
 */

/* Color aliases */
//#define C0 VGA_BLACK
//#define C1 VGA_RAND01
//#define C2 VGA_RAND04
//#define C3 VGA_RAND02
//#define C4 VGA_RAND03
    const color_t C0 = VGA_BLACK;
    const color_t C1 = randomColor();
    const color_t C2 = randomColor();
    const color_t C3 = randomColor();
    const color_t C4 = randomColor();
       

#define GO_WIDTH  17
#define GO_HEIGHT 13

#define GO_EMPTY_LINE {C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0},

    const char drawing[GO_HEIGHT][GO_WIDTH] = {
        GO_EMPTY_LINE

        {C0, C0, C1, C1, C0, C0, C2, C0, C0, C3, C0, C3, C0, C4, C4, C4, C0},
        {C0, C1, C0, C0, C0, C2, C0, C2, C0, C3, C3, C3, C0, C4, C0, C0, C0},
        {C0, C1, C1, C1, C0, C2, C0, C2, C0, C3, C0, C3, C0, C4, C4, C0, C0},
        {C0, C1, C0, C1, C0, C2, C2, C2, C0, C3, C0, C3, C0, C4, C0, C0, C0},
        {C0, C0, C1, C1, C0, C2, C0, C2, C0, C3, C0, C3, C0, C4, C4, C4, C0},

        GO_EMPTY_LINE

        {C0, C0, C4, C0, C0, C3, C0, C3, C0, C2, C2, C2, C0, C1, C1, C1, C0},
        {C0, C4, C0, C4, C0, C3, C0, C3, C0, C2, C0, C0, C0, C1, C0, C1, C0},
        {C0, C4, C0, C4, C0, C3, C0, C3, C0, C2, C2, C0, C0, C1, C0, C1, C0},
        {C0, C4, C0, C4, C0, C3, C0, C3, C0, C2, C0, C0, C0, C1, C1, C0, C0},
        {C0, C0, C4, C0, C0, C0, C3, C0, C0, C2, C2, C2, C0, C1, C0, C1, C0},

        GO_EMPTY_LINE
    };

    coord_t y;
    coord_t x;

    coord_t base_x = VGA_COL_START+(VGA_COL_END-VGA_COL_START-GO_WIDTH)/2;
    coord_t base_y = VGA_ROW_START+1+(VGA_ROW_END-VGA_ROW_START-GO_WIDTH)/2;

    for (y = 0; y < GO_HEIGHT; y++) {
        for (x = 0; x < GO_WIDTH; x++) {
            VGA_SetPixelXY(base_x+x, base_y+y, drawing[y][x]);
        }
    }
}

/**
 * Print current speed
 *
 */
void printSpeed(const unsigned short speed) {
/* Firstly we need to define matrix to draw.
 * To preserve compact format, some macros are used.
 */

/* Color aliases */
#define S0 VGA_BLACK
#define S1 VGA_FALLEN

#define SPD_WIDTH  19
#define SPD_HEIGHT 5

    const color_t speedDrawing[SPD_HEIGHT][SPD_WIDTH] = {
        {S0, S1, S1, S0, S1, S1, S1, S0, S1, S1, S1, S0, S1, S1, S1, S0, S1, S1, S0}, 
        {S1, S0, S0, S0, S1, S0, S1, S0, S1, S0, S0, S0, S1, S0, S0, S0, S1, S0, S1}, 
        {S1, S1, S0, S0, S1, S1, S1, S0, S1, S1, S0, S0, S1, S1, S0, S0, S1, S0, S1}, 
        {S0, S0, S1, S0, S1, S0, S0, S0, S1, S0, S0, S0, S1, S0, S0, S0, S1, S0, S1}, 
        {S1, S1, S0, S0, S1, S0, S0, S0, S1, S1, S1, S0, S1, S1, S1, S0, S1, S1, S0}, 
    };

#define NUM_COUNT 10
#define NUM_WIDTH 3
#define NUM_HEIGHT 5
    const color_t digit[NUM_COUNT][NUM_HEIGHT][NUM_WIDTH] = {
        {
            {S0, S1, S0},
            {S1, S0, S1},
            {S1, S0, S1},
            {S1, S0, S1},
            {S0, S1, S0 },
        },
        {
            {S0, S0, S1},
            {S0, S1, S1},
            {S1, S0, S1},
            {S0, S0, S1},
            {S0, S0, S1},
        },
        {
            {S0, S1, S0},
            {S1, S0, S1},
            {S0, S0, S1},
            {S0, S1, S0},
            {S1, S1, S1},
        },
        {
            {S1, S1, S0},
            {S0, S0, S1},
            {S0, S1, S0},
            {S0, S0, S1},
            {S1, S1, S0},
        },
        {
            {S1, S0, S0},
            {S1, S0, S1},
            {S1, S1, S1},
            {S0, S0, S1},
            {S0, S0, S1},
        },
        {
            {S1, S1, S1},
            {S1, S0, S0},
            {S1, S1, S0},
            {S0, S0, S1},
            {S1, S1, S0},
        },
        {
            {S1, S1, S1},
            {S1, S0, S0},
            {S1, S1, S1},
            {S1, S0, S1},
            {S1, S1, S1},
        },
        {
            {S1, S1, S1},
            {S0, S0, S1},
            {S0, S1, S0},
            {S1, S0, S0},
            {S1, S0, S0},
        },
        {
            {S0, S1, S0},
            {S1, S0, S1},
            {S0, S1, S0},
            {S1, S0, S1},
            {S0, S1, S0},
        },
        {
            {S1, S1, S1},
            {S1, S0, S1},
            {S1, S1, S1},
            {S0, S0, S1},
            {S0, S0, S1},
        },
    };

    coord_t y;
    coord_t x;

    //SPEED drawing
    for (y = 0; y < SPD_HEIGHT; y++) {
        for (x = 0; x < SPD_WIDTH; x++) {
            VGA_SetPixelXY(VGA_SPEED_COL_START+x, VGA_SPEED_ROW_START+y, speedDrawing[y][x]);
        }
    }

    const unsigned short num_base_x = VGA_SPEED_COL_START-NUM_WIDTH/2+SPD_WIDTH/2;
    const unsigned short num_base_y = VGA_SPEED_ROW_START+1+SPD_HEIGHT+1;
    color_t color = speed % VGA_RAND_COUNT + VGA_RAND_START;
    if (speed == 0) {
        color = VGA_FALLEN;
    }

    //clear previous number
    for (y = 0; y < NUM_HEIGHT; y++) {
        for (x = 0; x < NUM_WIDTH; x++) {
            VGA_SetPixelXY(num_base_x + x, num_base_y + y, VGA_BLACK);
        }
    }

    //And choose the correct number
    for (y = 0; y < NUM_HEIGHT; y++) {
        for (x = 0; x < NUM_WIDTH; x++) {
            VGA_SetPixelXY(num_base_x + x, num_base_y + y, digit[speed][y][x] == S1 ? color : VGA_BLACK);
        }
    }
}

/**
 * Print nextBlock
 *
 */
void printNextBlock(const block_t block, const color_t color) {
/* Firstly we need to define matrix to draw.
 * To preserve compact format, some macros are used.
 */

/* Color aliases */
#define N0 VGA_BLACK
#define N1 VGA_FALLEN

#define NXT_WIDTH  15
#define NXT_HEIGHT 5

    const color_t nextDrawing[SPD_HEIGHT][SPD_WIDTH] = {
        {N1, N0, N1, N0, N1, N1, N1, N0, N1, N0, N1, N0, N1, N1, N1}, 
        {N1, N0, N1, N0, N1, N0, N0, N0, N1, N0, N1, N0, N0, N1, N0}, 
        {N1, N0, N1, N0, N1, N1, N0, N0, N0, N1, N0, N0, N0, N1, N0}, 
        {N1, N1, N1, N0, N1, N0, N0, N0, N1, N0, N1, N0, N0, N1, N0}, 
        {N1, N0, N1, N0, N1, N1, N1, N0, N1, N0, N1, N0, N0, N1, N0}, 
    };

    coord_t y;
    coord_t x;

    //NEXT drawing
    for (y = 0; y < NXT_HEIGHT; y++) {
        for (x = 0; x < NXT_WIDTH; x++) {
            VGA_SetPixelXY(VGA_NEXT_COL_START+x, VGA_NEXT_ROW_START+y, nextDrawing[y][x]);
        }
    }

    //erase the palce for block
    const coord_t block_base_x = VGA_NEXT_COL_START-BLOCK_WIDTH+NXT_WIDTH/2+1;
    const coord_t block_base_y = VGA_NEXT_ROW_START+1+NXT_HEIGHT+1;

    for (y = 0; y < 2*BLOCK_HEIGHT; y++) {
        for (x = 0; x < 2*BLOCK_WIDTH; x++) {
            VGA_SetPixelXY(block_base_x+x, block_base_y+y, VGA_BLACK);
        }
    }

    printBlock(block_base_x, block_base_y, block, color, false);
}

/**
 * Constants for sampling keyboard
 */
#define UNPRESSED_KEY 0x0000
#define PRESSED_LONG_ENOUGH 0x0851

/**
 * Set value for the key to current value of TAR.
 *
 * Caution: TAR can't be UNPRESSED_KEY because it is a special value.
 * If so, just increase value.
 *
 * @param   since - time to update
 */
static inline void setPressedTime(uint16_t *since) {
    *since = TAR;
    if (*since == UNPRESSED_KEY) {
        *since += 1;
    }
}

/**
 * Handle a keyboard
 *
 * @param   keyboardState - bit vector of pressed keys
 * @param   key - key to test
 * @param   action - result action (if pressed)
 * @param   since - last time the key was set
 * @return  action to do (ACT_NONE if nothing has happend)
 */
static inline unsigned char handleKey(const unsigned short keyboardState, const unsigned short key, const unsigned char action, uint16_t *since) {
    if (keyboardState & key) { //pressed key
        //key is still pressed
        
        if (*since == UNPRESSED_KEY) { //first time pressed key
            //first time pressed key
            setPressedTime(since);
            return ACT_NONE;
        }
        else if (TAR - *since > PRESSED_LONG_ENOUGH) {
            //key is pressed long enough -> call action
            setPressedTime(since);
            return action;
        }
    }
    else if (*since != UNPRESSED_KEY && TAR - *since > PRESSED_LONG_ENOUGH) {
        //key is not pressed but it was hold long enough
        *since = UNPRESSED_KEY;
        return action;
    }
    else {
        //key is not pressed and it wasn't hold for long enough
        *since = UNPRESSED_KEY;
        return ACT_NONE;
    }

    return ACT_NONE;
}

/**
 * Get current status of keyboard
 *
 * @return  bit mask of active keys
 */
unsigned char kbstate(void) {
    static uint16_t pressedSince[10] = {UNPRESSED_KEY, UNPRESSED_KEY, UNPRESSED_KEY, UNPRESSED_KEY, UNPRESSED_KEY};

    unsigned short keyboardState = read_word_keyboard_4x4();
    unsigned char result = ACT_NONE;

    if (!startNewGame) // stisk libovolne klavesy zahaji hru
    {
        // stisk 0-9 ma vyznam nastaveni levelu na pocatku hry (pocet nahodnych radku)
        levelInNewGame = (unsigned int) key_decode(keyboardState) - '0';
        if (levelInNewGame > 9) levelInNewGame = 0;
        startNewGame = keyboardState;   // cokoliv nenuloveho = true (stisk libovolne klavesy)
        return ACT_NONE;
    }
    
    result |= handleKey(keyboardState, KEY_4, ACT_LEFT, &pressedSince[0]);
    result |= handleKey(keyboardState, KEY_6, ACT_RIGHT, &pressedSince[1]);
    result |= handleKey(keyboardState, KEY_2, ACT_ROTATE, &pressedSince[2]);
    result |= handleKey(keyboardState, KEY_5, ACT_ROTATE, &pressedSince[3]);
    result |= handleKey(keyboardState, KEY_8, ACT_DOWN, &pressedSince[4]);

    return result;
}

/**
 * Write point status to LCD
 */
void LCD_WritePoints(const unsigned points) {
    static char lcd_str[20];

    if (points == 0) {
        sprintf(lcd_str, "no line");
    }
    else if (points == 1) {
        sprintf(lcd_str, "1 line");
    }
    else {
        sprintf (lcd_str, "%d lines", points);
    }

    LCD_write_string(lcd_str);
}
