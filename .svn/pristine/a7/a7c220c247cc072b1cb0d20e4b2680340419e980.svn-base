//IMP project, Izidor Matusov, xmatus19, inspired by main.c from snake/, my work cca 98%
//inspired by timers and their mechanism
//
/*******************************************************************************
   main.c: Tetris game
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fitkitlib.h>
#include <lcd/display.h>
#include <thermometer/thermometer.h>

#include "tetris.h"
#include "types.h"
#include "utils.h"

/**
 * Timer's stuff
 */
unsigned int falling_delay; // delay between moving a block down
bool fall_flag = false;

/**
 * Set flag to move the block one point lower
 */
interrupt (TIMERA0_VECTOR) fallingDownTimer(void) {
    CCR0 += falling_delay; 
    fall_flag = true;
    flip_led_d5();
}

/**
 * Global variables which influence the game
 */
unsigned short speed = 0;
unsigned short levelInNewGame = 0;
bool gameOver = true;
bool startNewGame = false;

/**
 * Generate parameters for a new block
 */
static inline void newBlock(coord_t *x, coord_t *y, block_t *block, color_t *color) {
    *x = (DIM_WIDTH - BLOCK_WIDTH)/2;
    *y = 0;
    *block = randomBlock();
    *color = randomColor();
}

/**
 * Uniform way of change speed
 */
static inline void setSpeed(const unsigned short newSpeed) {
    if (speed < SPEEDS_COUNT) {
        speed = newSpeed;
    }
    else {
        speed = 5;
    }
    falling_delay = speeds[speed].delay;
    printSpeed(speed);
}

/**
 * Function for playing a tetris game
 *
 * Because of setting parameters from the terminal, 
 * most of variables have to be global :-(
 */
void play(const unsigned level) {
    generateBoard(level);

    coord_t x;
    coord_t y;
    block_t block = randomBlock();
    color_t color = randomColor();;
    block_t nextBlock;
    color_t nextColor;

    newBlock(&x, &y, &nextBlock, &nextColor);
    printBlock(x,y, block, color, true);
    printNextBlock(nextBlock, nextColor);

    setSpeed(speed);

    unsigned short pointsToNextSpeed = 0;
    unsigned short totalPoints = 0;
    LCD_WritePoints(totalPoints);

    gameOver = false;
    startNewGame = false;

    falling_delay = 0xFFFF;
    /* Enable timer */
    CCTL0 = CCIE;
    CCR0 = falling_delay;
    TACTL = TASSEL_1 + MC_2;

    set_led_d5(0);

    /* Game begins! */
    while (!gameOver)  {
        terminal_idle();

        /* Handling keyboard actions */
        unsigned short actions = kbstate();
        if (actions != ACT_NONE) {
            eraseBlock(x, y, block, true);

            if (actions & ACT_LEFT) {
                if (isPossiblePlace(x-1, y, block)) {
                    x -= 1;
                }
            }

            if (actions & ACT_RIGHT) {
                if (isPossiblePlace(x+1, y, block)) {
                    x += 1;
                }
            }

            if (actions & ACT_DOWN) {
                if (isPossiblePlace(x, y+1, block)) {
                    y += 1;
                }
            }

            if (actions & ACT_ROTATE) {
                block_t newBlock;
                newBlock = rotateBlock(block);
                if (isPossiblePlace(x, y, newBlock)) {
                    block = newBlock;
                }
            }

            printBlock(x,y, block, color, true);
        }

        /* Let block fall down */
        if (fall_flag) {
            eraseBlock(x, y, block, true);

            if (isPossiblePlace(x, y+1, block)) {
                y++;
            }
            else { //Block hits the bottom
                placeBlock(x, y, block);

                unsigned short newPoints = removeFullLines();
                pointsToNextSpeed += newPoints;
                totalPoints += newPoints;
                LCD_WritePoints(totalPoints);

                if (pointsToNextSpeed > speeds[speed].pointsToNextSpeed) {
                    if(speed+1 < SPEEDS_COUNT) {
                        setSpeed(speed+1);

                        term_send_str("Higher speed: ");
                        term_send_num(speed);
                        term_send_str_crlf("!");
                    }

                    pointsToNextSpeed = 0;
                }

                block = nextBlock;
                color = nextColor;

                newBlock(&x, &y, &nextBlock, &nextColor);
                if (!isPossiblePlace(x,y, block)) {
                    gameOver = true;
                    startNewGame = false;   // aby bylo mozne klavesou spustit dalsi hru
                    printGameOver();
                }

                printNextBlock(nextBlock, nextColor);
            }

            fall_flag = false;
            printBlock(x,y, block, color, true);
        }

    } /* Game over :-( */

    /* Disable timer */
    CCTL0 = 0;
    set_led_d5(0);

    /* Setting speed back to normal */
    speed = 0;

    term_send_str_crlf("Game is over");
    term_send_str("Final score: ");
    term_send_num(totalPoints);
    term_send_str_crlf("");
}

/**
 * Prints help for user
 */
void print_user_help(void) {
    term_send_str_crlf(" START .......... begin a new game");
    term_send_str_crlf(" RESTART ........ restart the game");
    term_send_str_crlf(" STOP ........... finish the current game");
    term_send_str_crlf(" SPEED 0-9 ...... set speed");
    term_send_str_crlf(" LEVEL 0-15 ..... set startup level for new game");
}

/**
 * Decodes an user command and changes the appropriate global variables
 *
 * @see     print_user_help()
 * @param   cmd_ucase - user command in unified case
 * @param   cmd - real user command
 * @return  USER_COMMAND for matched command, CMD_UNKNOWN otherwise
 */
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) {
    if (strcmp5(cmd_ucase, "START")) {
        if (gameOver) {
            startNewGame = true;

            term_send_str_crlf("A new game begins");
        }
        else {
            term_send_str_crlf("Game is already started");
        }

        return USER_COMMAND;
    }
    else if (strcmp7(cmd_ucase, "RESTART")) {
        gameOver = true;
        startNewGame = true;

        term_send_str_crlf("A new game starts");
        return USER_COMMAND;
    }
    else if (strcmp4(cmd_ucase, "STOP")) {
        gameOver = true;

        term_send_str_crlf("Stopping the game");
        return USER_COMMAND;
    }
    else if (strcmp6(cmd_ucase, "SPEED ")) {
        if ('0' <= cmd_ucase[6] && cmd_ucase[6] <= '9') {
            setSpeed(cmd_ucase[6] - '0');

            term_send_str("Setting speed to ");
            term_send_num(speed);
            term_send_str_crlf("");
            return USER_COMMAND;
        }
        else {
            return CMD_UNKNOWN;
        }
    }
    else if (strcmp6(cmd_ucase, "LEVEL ")) {
        if ('0' <= cmd_ucase[6] && cmd_ucase[6] <= '9') {
            levelInNewGame = cmd_ucase[6] - '0';

            if (levelInNewGame == 1 && '0' <= cmd_ucase[7] && cmd_ucase[7] <= '5') {
                levelInNewGame = 10 + cmd_ucase[7] -'0';
            }

            term_send_str("Level in a next game will be ");
            term_send_num(levelInNewGame);
            term_send_str_crlf("");
            return USER_COMMAND;
        }
        else {
            return CMD_UNKNOWN;
        }
        return USER_COMMAND;
    }
    else {
        print_user_help();
    }

    return CMD_UNKNOWN;
}

/**
 * This function is called after initialization of FPGA.
 *
 * It draw first screen and initialize LCD
 */
void fpga_initialized(void) {
    VGA_DrawScene();
    LCD_init();
    LCD_write_string("Tetris game");
}

/**
 * Main function
 *
 * The fun begins there :-)
 */
int main(void) {
    /* Initialization */
    initialize_hardware();
    thermometer_init_rand();
    WDG_stop();
    set_led_d5(0);

    term_send_str_crlf("How to control this app:");
    print_user_help();

    while (1) {
        terminal_idle();

        if (startNewGame) {
            VGA_DrawScene();
            play(levelInNewGame);
        } else kbstate();
    }
} 
