/*******************************************************************************
   sokoban.h: Sokoban - functions
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

#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>
#include <stdio.h>
#include <string.h>
#include "font_texture.h"
#include "sokoban.h"

// vektory pohybu hrace
T_move player_move[] = {{0,-1},{0,1},{-1,0},{1,0}};


//Zjisteni stavu klavesnice
unsigned short keyboard_state(void) {
  unsigned short keyst = read_word_keyboard_4x4();

  if (keyst & KEY_4) return CMD_LEFT;
  if (keyst & KEY_6) return CMD_RIGHT;
  if (keyst & KEY_2) return CMD_UP;
  if (keyst & KEY_8) return CMD_DOWN;
  if (keyst & KEY_5) return CMD_ENTER;
  if (keyst & KEY_m) return CMD_RESTART;

  return CMD_NONE;
}
/*******************************************************************************
 * Funkce pro statistiky
*******************************************************************************/ 
// funkce pro inicializaci statistik
void sokoban_statistics_clear()
{
  int i;
  unsigned char buf[] = {255, 255, 255};
    
  for (i = 0; i < 6; i++)
    sokoban_update_statistics(i, buf);

  term_send_str_crlf("Statistiky vynulovany");

}
// funkce pro ziskani nejlepsiho casu hry
void sokoban_get_statistics(unsigned char level, T_time *get_time)
{
  unsigned char buf[3];
  long addr;
  // cas kazde hry ma velikost ve flash 3 byty
  addr = FLASH_PageToAddr(STATISTICS_PAGE ) + level * 3; //base + offset
  FLASH_ReadDataA((unsigned char *)&buf, 3, addr);
  
  get_time->h = buf[0];
  get_time->m = buf[1];
  get_time->s = buf[2];    
}

// funkce pro aktualizaci statistik
void sokoban_update_statistics(unsigned char level, unsigned char *time)
{
  long addr;
  int i;
  // cas kazde hry ma velikost ve flash 3 byty 
  addr = FLASH_PageToAddr(STATISTICS_PAGE) + level * 3; //base + offset
  for (i = 0; i < 3; i++)
    FLASH_WriteByte(addr + i, time[i]);

}
/*******************************************************************************
 * Funkce pro inicializaci a hrani hry
*******************************************************************************/
// funkce zabezpecujici pohyb hrace
void sokoban_player_move(unsigned char *x, unsigned char *y, short int move)
{
  unsigned char tex_id, tex_id1;
  unsigned char n_pos_x, n_pos_y;
  n_pos_x = *x + player_move[move].x;
  n_pos_y = *y + player_move[move].y;
  // kontrola zda jsme na obrazovce
  if ((on_board(n_pos_x , n_pos_y)) == 1)
  {// zjistim co se nachazi na miste kam se chco posouvat
    texture_indexmap_read(n_pos_x, n_pos_y, &tex_id);
    
    if ((tex_id&3) == 1)
    {// je tam volna pozice muzu se presunout
      *x = n_pos_x;
      *y = n_pos_y;
      texture_set(*x, *y, 2);    
    }
    else if((tex_id&3) == 3)
    {//je tam bedna, takze zkontroluju co je na dalsi pozici pred bednou
      texture_indexmap_read(*x + player_move[move].x * 2, 
                         *y + player_move[move].y * 2, &tex_id1);
      if((tex_id1&3) == 1)
      {//je tam volno takze bedna i hrac se presunou
        *x += player_move[move].x;
        *y += player_move[move].y;
        texture_set(*x, *y, 2);
        texture_set(*x + player_move[move].x, *y + player_move[move].y, 3);
      }
      else
      {// neni tam volno (dalsi bedna nebo konec hraci desky) 
       // takze se neposouvam
        texture_set(*x, *y, 2);
      }
    }
    else if ((tex_id&3) == 0) 
    {// mimo hraci plochu, neposouvam se
      texture_set(*x, *y, 2);    
    }
  }
  else 
  {// mimo obrazovku neposouvam se
    texture_set(*x, *y, 2);    
  }
}

// spocita pocet beden na koncovych pozicich
unsigned char sokoban_check_end()
{
  unsigned char id;
  unsigned char x, y;
  unsigned char ch = 0;

  for (y=0; y < TEXTURE_ROWS; y++) 
    for (x=0; x < TEXTURE_COLUMNS; x++) 
        {
          texture_indexmap_read(x, y, &id);
          if (id == 7) {
            ch++;
          }
        }
  return ch;
}

void sokoban_game_info(unsigned char level, T_level_info *info)
{
  unsigned char buf[3];
  long addr;
  addr = FLASH_PageToAddr(LEVEL_PAGE) + level * 303; //base + offset
  FLASH_ReadDataA((unsigned char *)&buf, 3, addr);
  info->x = buf[0];
  info->y = buf[1];
  info->boxes = buf[2];
}

/*******************************************************************************
 * Funkce pro mereni casu hry
*******************************************************************************/
// Obsluha preruseni casovace timer A0
interrupt (TIMERA0_VECTOR) Timer_A (void)
{
  // nastaveni po kolika ticich (32768 = 0x8000, tj. za 1 s) 
  //ma dojit k dalsimu preruseni
  CCR0 += 0x8000;   

  game_time.s++; 

  if(game_time.s==60) {game_time.m++; game_time.s=0;}
  if(game_time.m==60) {game_time.h++; game_time.m=0;}
  if(game_time.h==24) {game_time.h = game_time.m = game_time.s = 0;}
  
  display_time();
}

// spusti odpocitavani casu hry
void spusteni_casu()
{
  // vynulovani
  game_time.s=0;     
  game_time.m=0;  
  game_time.h=0;
  // povol preruseni pro casovac (rezim vystupni komparace) 
  CCTL0 = CCIE;
  // nastaveni po kolika ticich (32768 = 0x8000, tj. za 1 s) 
  //ma dojit k dalsimu preruseni                           
  CCR0 = 0x8000;
  // nastaveni casovace - ACLK (f_tiku = 32768 Hz = 0x8000 Hz)
  // nepretrzity rezim                  
  TACTL = TASSEL_1 + MC_2;
}
// zastavi cas 
void zastaveni_casu()
{
  TACTL = MC_0 + TACLR;         // zastaveni casovace a jeho vynulovani                    
  CCTL0 = !CCIE;                // vypnuti preruseni
}
// Zobrazeni casu a datumu na LCD 
void display_time() {
    // Vypis aktualniho casu na displej
    LCD_write_string("Cas   :");
    // kurzor na druhou polovinu
    LCD_send_cmd(LCD_SET_DDRAM_ADDR | LCD_SECOND_HALF_OFS, 0); 
    delay_ms(2);
    LCD_send_cmd((unsigned char)(game_time.h/10)+48, 1); delay_ms(2);
    LCD_send_cmd((unsigned char)(game_time.h%10)+48, 1); delay_ms(2); 
    LCD_send_cmd(0x3A, 1); delay_ms(2);
    LCD_send_cmd((unsigned char)(game_time.m/10)+48, 1); delay_ms(2);
    LCD_send_cmd((unsigned char)(game_time.m%10)+48, 1); delay_ms(2);     
    LCD_send_cmd(0x3A, 1); delay_ms(2);
    LCD_send_cmd((unsigned char)(game_time.s/10)+48, 1); delay_ms(2);
    LCD_send_cmd((unsigned char)(game_time.s%10)+48, 1); delay_ms(2);
}
/*******************************************************************************
 * Funkce pro inicializaci obrazovek
*******************************************************************************/
// inicializace obrazovky s vyberem urovni hry
void sokoban_level_init(void)
{
  unsigned char i;
  VGA_Clear();
  VGA_TextOut(36, 3, "Zvolte hru");
  VGA_TextOut(35, 4, "------------");
  
  for (i = 0; i < 6; i++)
    print_item(i + 6);
  
  select_item(6);
}
// Inicializace obrazkovy o napovedy
void sokoban_help_init(void)
{
  VGA_Clear();
  VGA_TextOut(36, 3, "Napoveda");
  VGA_TextOut(35, 4, "----------");
  VGA_TextOut(10,10, "Ukolem hry je presunout bedny na vyznacene pozice.");
  VGA_TextOut(10,11, "Hra se ovlada pohybovymi klavesami 2,4,6,8.");
  VGA_TextOut(10,12, "Klavesa 5 slouzi k vybrani polozky menu ");
  VGA_TextOut(10,13, "nebo navratu z jine obrazovky do menu.");
  VGA_TextOut(10,14, "Pokud jste ve hre muzete hru predcasne ukoncit");
  VGA_TextOut(10,15, "klavesou #.");
  print_item(4);
  select_item(4);
}
// Inicializace obrazkovy statistik
void sokoban_statistics_init(void)
{
  int i;
  T_time time;
  unsigned char print_buf[30];
  VGA_Clear();
  VGA_TextOut(36, 3, "Statistiky");
  VGA_TextOut(35, 4, "------------");
  VGA_TextOut(35, 4, "");
  // vypis obsahu Flash na obrazovku
  for(i = 0; i < 6; i++)
  {
    sokoban_get_statistics((char)i, &time);
    sprintf(print_buf,"Level %d     Cas:%02d:%02d:%02d", i + 1, time.h,
            time.m, time.s);
    VGA_TextOut(26, 8 + i*2, print_buf);
  }
  print_item(5);
  print_item(4);
  select_item(4);
}
// Inicializace obrazkovy konec hry 
void sokoban_end_game_init(unsigned char level, T_time game_time, unsigned char fail)
{
  unsigned char buf[30];
  unsigned char best_time[20];
  unsigned char time[20];
  T_time get_time;
  VGA_Clear();
  VGA_TextOut(36, 3, "Konec hry");
  VGA_TextOut(35, 4, "------------");
  if(fail == 1)
    VGA_TextOut(35, 14, "Prohral jste");
  else
  {
    sokoban_get_statistics(level, &get_time);
    sprintf(best_time, "%02d:%02d:%02d\n" , get_time.h, get_time.m, get_time.s);
    sprintf(time, "%02d:%02d:%02d\n" , game_time.h, game_time.m, game_time.s);
    // porovnani nejlepsiho casu a aktualniho dosazeneho casu
    if (strcmp(best_time, time) > 0)
    {
      VGA_TextOut(31, 15, "Novy nejlepsi cas!");
      sprintf(buf, "Cas hry: %s", time);
      VGA_TextOut(31, 17, buf);
      
      buf[0] = game_time.h;
      buf[1] = game_time.m;
      buf[2] = game_time.s;
      sokoban_update_statistics(level, buf);
    }
    else
    {
      sprintf(buf, "Nejlepsi cas hry: %s", best_time);
      VGA_TextOut(25, 15, buf);
      sprintf(buf, "Cas hry: %s", time);
      VGA_TextOut(30, 17, buf);
    }
  }
  print_item(4);
  select_item(4);
}
// Inicializace obrazkovy o aplikaci
void sokoban_about_init(void)
{
  VGA_Clear();
  VGA_TextOut(36, 3, "O aplikaci");
  VGA_TextOut(35, 4, "------------");
  VGA_TextOut(20, 10, "Tato aplikace vznikla jakou soucast BP");
  VGA_TextOut(20, 11, "Autor: Jiri Subr");
  VGA_TextOut(20, 12, "mail: xsubrj00@stud.fit.vutbr.cz");
  
  print_item(4);
  select_item(4);
}

// Inicializace Menu
void sokoban_menu_init(unsigned char sel_item)
{
  unsigned char i;
  unsigned char c;
  unsigned char text[20];

  VGA_Clear();
  
  sprintf(text,"SOKOBAN");
  VGA_TextOut(36, 3, text);
  c = 98;
  font_indexmap_write(39, 14, &c);
  
  items_init();
  
  for (i = 0; i < 4; i++)
    print_item(i);
  
  select_item(sel_item);
}
