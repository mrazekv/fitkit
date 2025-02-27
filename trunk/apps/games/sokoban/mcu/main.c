/*******************************************************************************
   main.c: Sokoban - main application
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

#include <string.h>
#include <stdio.h>

#include <fitkitlib.h>
#include <lcd/display.h>
#include <keyboard/keyboard.h>

#include "sokoban.h"
#include "font_texture.h"

#define MENU_ITEMS  4               // pocet polozek menu
   
screen_type scr;                    // zobrazovana obrazovka
unsigned char mode;                 // mod zobrazeni
unsigned char cursor_pos = 0;       // pozice kursoru v textovych obrazovkach
unsigned char cursor_pos_menu = 0;  // zapamatovani predchazejici pozice v menu
T_level_info game_info;             // informace o hre
unsigned short command;             // aktualni prikaz
// externi struktura ve ktere je cas hry
extern T_time game_time;
// funkce pro zpracovani prikazu menu
void sokoban_menu(short int cmd)
{
  cursor_pos = cursor_pos_menu;
  
  // pohyb po menu (po smeru/proti smeru hod. rucicek)
  if (cmd == CMD_UP || cmd == CMD_LEFT) {
    unselect_item(cursor_pos);
    cursor_pos = (cursor_pos == 0)? 3 : cursor_pos - 1;
    select_item(cursor_pos);
  }
  if (cmd == CMD_DOWN || cmd == CMD_RIGHT) {
    unselect_item(cursor_pos);
    cursor_pos = (cursor_pos == 3)? 0 : cursor_pos + 1;
    select_item(cursor_pos);
  }
  // Info vypisy na displeji
  if(cursor_pos == 0) LCD_write_string("Hra");
  if(cursor_pos == 1) LCD_write_string("Napoveda");
  if(cursor_pos == 2) LCD_write_string("O aplikaci");
  if(cursor_pos == 3) LCD_write_string("Statistiky");
  
  cursor_pos_menu = cursor_pos;
  // Vyberem jednu polozku z menu
  if (cmd == CMD_ENTER) {
    if(cursor_pos == 0) {scr = SCR_LEVEL; sokoban_level_init();}
    if(cursor_pos == 1) {scr = SCR_HELP; sokoban_help_init();}
    if(cursor_pos == 2) {scr = SCR_ABOUT; sokoban_about_init();}
    if(cursor_pos == 3) {scr = SCR_STATISTICS; sokoban_statistics_init();}
    cursor_pos = 0;
  }
}
// funkce pro zpracovani prikazu vlastni hry
void sokoban_game(short int cmd)
{
  if (cmd < 4 && cmd >= 0)
  {// pohyb hrace
    position_clear(game_info.x, game_info.y);     // vymazeme puvodni polohu
    sokoban_player_move(&game_info.x, &game_info.y, cmd); // nova poloha hrace
    if(sokoban_check_end() == game_info.boxes)
    {//konec hry, zobrazeni nejlepsiho casu a prave dosahnuteho
      zastaveni_casu();
      mode = 0;
      mode_write(&mode);
      sokoban_end_game_init(cursor_pos, game_time, 0);
      scr = SCR_ENDGAME;
      return;
    }
  }
  else if (cmd == CMD_RESTART) 
  {//predcasne ukonceni hry
    zastaveni_casu();
    sokoban_end_game_init(cursor_pos, game_time, 1);
    mode = 0;
    mode_write(&mode);
    
    scr = SCR_ENDGAME;
  }
}
// funkce pro zpracovani prikazu obrazovky s urovnemi
void sokoban_level(short int cmd)
{
  // vyber levelu, pohyb po menu
  if (cmd == CMD_UP || cmd == CMD_LEFT) {
    unselect_item(cursor_pos + 6);
    cursor_pos = (cursor_pos == 0)? 5 : cursor_pos - 1;
    select_item(cursor_pos + 6);
  }
  if (cmd == CMD_DOWN || cmd == CMD_RIGHT) {
    unselect_item(cursor_pos + 6);
    cursor_pos = (cursor_pos == 5)? 0 : cursor_pos + 1;
    select_item(cursor_pos + 6);
  }
  // Info vypisy na displeji
  if(cursor_pos == 0) LCD_write_string("Level 1");
  if(cursor_pos == 1) LCD_write_string("Level 2");
  if(cursor_pos == 2) LCD_write_string("Level 3");
  if(cursor_pos == 3) LCD_write_string("Level 4");
  if(cursor_pos == 4) LCD_write_string("Level 5");
  if(cursor_pos == 5) LCD_write_string("Level 6");
  if (cmd == CMD_ENTER) {
    mode = 1;                 // nastavime graficky rezim na zobrazovani textur
    mode_write(&mode);
    cursor_pos = cursor_pos;
    sokoban_game_info(cursor_pos, &game_info);
    init_board(cursor_pos);           // nacitame vybranou hru
    spusteni_casu();                  // spustime cas
    scr = SCR_GAME;
  }
}
// funkce pro zpracovani prikazu obrazovky napovedy
void sokoban_help(short int cmd)
{
  if (cmd == CMD_ENTER) {
    sokoban_menu_init(1);
    scr = SCR_MENU;
  }
}
// funkce pro zpracovani prikazu obrazovky o aplikaci
void sokoban_about(short int cmd)
{
  if (cmd == CMD_ENTER) {
    sokoban_menu_init(2);
    scr = SCR_MENU;
  }
}
// funkce pro zpracovani prikazu obrazovky statistiky
void sokoban_statistics(short int cmd)
{
  // vyber levelu, pohyb po menu
  if (cmd == CMD_UP || cmd == CMD_LEFT) {
    unselect_item(cursor_pos + 4);
    cursor_pos = (cursor_pos == 0)? 1 : cursor_pos - 1;
    select_item(cursor_pos + 4);
  }
  if (cmd == CMD_DOWN || cmd == CMD_RIGHT) {
    unselect_item(cursor_pos + 4);
    cursor_pos = (cursor_pos == 1)? 0 : cursor_pos + 1;
    select_item(cursor_pos + 4);
  }
  if (cmd == CMD_ENTER) {
    if(cursor_pos == 0) {scr = SCR_MENU; sokoban_menu_init(3);}
    if(cursor_pos == 1)
    {
      scr = SCR_STATISTICS;
      sokoban_statistics_clear();
      sokoban_statistics_init();
    }
    cursor_pos = 0;
  }
}
// funkce pro zpracovani prikazu obrazovky konec hry
void sokoban_end_game(short int cmd)
{
  if (cmd == CMD_ENTER) {
    sokoban_menu_init(0);
    scr = SCR_MENU;
  }
}
//Restartuje hru
void sokoban_restart(void)
{
  scr = SCR_MENU;
  cursor_pos = 0;
  cursor_pos_menu = 0;
  sokoban_menu_init(0);
  mode = 0;
  mode_write(&mode);
}
// volani funkci pro zpracovani prikazu 
void do_cmd(unsigned short command)
{
  if (scr == SCR_MENU) {
    sokoban_menu(command);
  } else if (scr == SCR_GAME) {
    sokoban_game(command);
  } else if (scr == SCR_LEVEL) {
    sokoban_level(command);
  } else if (scr == SCR_HELP) {
    sokoban_help(command);
  } else if (scr == SCR_ABOUT) {
    sokoban_about(command);
  } else if (scr == SCR_STATISTICS) {
    sokoban_statistics(command);
  } else if (scr == SCR_ENDGAME) {
    sokoban_end_game(command);
  }
}
/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
   term_send_str_crlf(" RESTART ... restart hry");
   term_send_str_crlf(" FLASH W TEX ... nacteni textury do FLASH");
   term_send_str_crlf(" UPDATE TEX ... nacteni textury do BRAM");
   term_send_str_crlf(" FLASH W FONT ... nacteni fontu do FLASH");
   term_send_str_crlf(" UPDATE FONT ... nacteni fontu do BRAM");
   term_send_str_crlf(" FLASH W GAMES ... nacteni her do FLASH");
}

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) 
{
    if (strcmp(cmd_ucase, "8") == 0) {
     do_cmd(CMD_UP);
  } else if (strcmp(cmd_ucase, "4") == 0) {
     do_cmd(CMD_LEFT);
  } else if (strcmp(cmd_ucase, "6") == 0) {
     do_cmd(CMD_RIGHT);
  } else if (strcmp(cmd_ucase, "2") == 0) {
     do_cmd(CMD_DOWN);
  } else if (strcmp(cmd_ucase, "5") == 0) {
     do_cmd(CMD_ENTER);
  } else if (strcmp(cmd_ucase, "0") == 0) {
     do_cmd(CMD_RESTART);
  } else if (strcmp(cmd_ucase, "RESTART") == 0) {
     sokoban_restart();
  } else if (strcmp(cmd_ucase, "FLASH W TEX") == 0) {
    //max 4224B
    FLASH_WriteFile(FLASH_USER_DATA_PAGE,"Textura (4096B)","*.bin",TEXTURE_SIZE_PAGE); 
  }  else if (strcmp(cmd_ucase, "FLASH W FONT") == 0) {
    FLASH_WriteFile(FLASH_USER_DATA_PAGE + TEXTURE_SIZE_PAGE,"Font (2048)","*.fnt",FONT_SIZE_PAGE); //max 2112B
  } else if (strcmp(cmd_ucase, "FLASH W GAMES") == 0) {
    FLASH_WriteFile(LEVEL_PAGE, "Hry (1818B)", "*.skb", LEVELS_SIZE_PAGE);
  } else if (strcmp(cmd_ucase, "UPDATE TEX") == 0) {
    Texture_Flash_FPGA();
  } else if (strcmp(cmd_ucase, "UPDATE FONT") == 0) {
    Font_Flash_FPGA();
  }  else {
    return (CMD_UNKNOWN);
  }
  return (USER_COMMAND);
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  keyboard_init();                      //inicializace klávesnice
  LCD_init();                           //inicializace displeje 
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0); 
  LCD_write_string("Sokoban ...");      // zobrazime info na displeji


  //inicializace bram texturou a fontem, ma byt tady nebo az pri prikazu restart
  Font_Flash_FPGA();
  Texture_Flash_FPGA();

  sokoban_restart();                    // inicializace promennych
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void) 
{ 
  initialize_hardware();          // pocatecni inicializace
  set_led_d5(1);                  // Port P1, negace bitu LED0
  set_led_d6(1);                  // Port P5, negace bitu LED1

  while (1)
  {
    command = keyboard_state();      // zjistime zda nebyla stisknuta klavesnice
    delay_ms(100);
  
    if (command != CMD_NONE)
    {//zavolani funkci pro obsluhu aktualni obrazovky
      do_cmd(command);
      flip_led_d6();
    }
    WDG_reset();                           // nulovani watchdogu
    terminal_idle();                       // obsluha terminalu
  }
}
