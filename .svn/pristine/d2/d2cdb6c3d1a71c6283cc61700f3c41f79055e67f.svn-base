/*******************************************************************************
   sokoban.h: Sokoban header file
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jiri Subr <xsubrj00 AT stud.fit.vutbr.cz>

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

#ifndef _SOKOBAN_H_
#define _SOKOBAN_H_

//Bazove adresy
#define BASE_ADDR_KEYBOARD  0x40 //bazova adresa klavesnice
#define BASE_ADDR_MODE      0x30 //bazova adresa modu

//Makra pro prenos dat pres SPI

#define mode_write(data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_MODE, data, 1, 1)
        
#define mode_read(data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, BASE_ADDR_MODE, data, 1, 1)

// Jednotlive ovladaci prikazy
#define CMD_UP      0
#define CMD_DOWN    1
#define CMD_LEFT    2 
#define CMD_RIGHT   3
#define CMD_NONE    4
#define CMD_ENTER   5
#define CMD_RESTART 9

// typy obrazovek
typedef enum {SCR_MENU, SCR_GAME, SCR_LEVEL, SCR_HELP, SCR_ABOUT,
              SCR_STATISTICS,SCR_ENDGAME}screen_type;

// struktura pro ulozeni casu
typedef struct time {
  unsigned char h;
  unsigned char m;
  unsigned char s;
}T_time;
// promenna pro ukladani casu
T_time game_time;

// struktura hry
typedef struct level_info {
  unsigned char boxes;
  unsigned char x;
  unsigned char y;
}T_level_info;
/*******************************************************************************
 * Funkce pro mereni casu hry
*******************************************************************************/ 
// zastaveni casu
void zastaveni_casu(void);
// spusteni casu
void spusteni_casu(void);
// zobrazeni casu
void display_time(void);
// obsluha preruseni od Timer A0
interrupt (TIMERA0_VECTOR) Timer_A (void);


// inicializace obrazovek
void sokoban_menu_init(unsigned char sel_item);
void sokoban_level_init(void);
void sokoban_about_init(void);
void sokoban_help_init(void);
void sokoban_statistics_init(void);
void sokoban_end_game_init(unsigned char level, T_time game_time,
                          unsigned char fail);

// funkce pro aktualizaci statistik
void sokoban_statistics_clear(void);
// funkce pro ziskani nejlepsiho casu hry
void sokoban_get_statistics(unsigned char level, T_time *get_time);
// funkce pro aktualizaci statistik
void sokoban_update_statistics(unsigned char level, unsigned char *time);
//zjisteni stavu klavesnice
unsigned short keyboard_state(void);
// kontrola konce hry
unsigned char sokoban_check_end();
// pohyb hrace po hraci plose
void sokoban_player_move(unsigned char *x, unsigned char *y, short int move);
// ziskani informaci o hre
void sokoban_game_info(unsigned char level, T_level_info *info);
#endif  // _SOKOBAN_H_
