/*******************************************************************************
   main.c: Tick tack toe - main application
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>
              Zdenek Vasicek <vasicek AT fit.vutbr.cz>

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lcd/display.h>
#include <irq/fpga_interrupt.h>
#include <serial/fpga_rs232.h>
#include <thermometer/thermometer.h>
#include "ticktacktoe.h"

unsigned char move; //cislo tahu
unsigned char pos_x; //X-ova pozice kurzoru
unsigned char pos_y; //Y-ova pozice kurzoru
unsigned char player_id = 0; //ID hrajiciho hrace
char det_kit = -1; //ID kitu -1 jeden kit, 0 prvni hrac, 1 druhy hrac
char endgame = 0; //Hra je ukoncena

//Provede akci
// serial - 0 prikaz prisel ze seriove linky, neposilat znovu zpet
//        - 1 prikaz prisel z klavesnice nebo z terminalu
void do_cmd(unsigned short cmd, unsigned short serial) 
{
  char ch;
  unsigned char id;
  char buf[20];
  char *sbuf;

  if ((cmd == CMD_LEFT) && (pos_x > 0) && (!endgame)) pos_x--;
  if ((cmd == CMD_RIGHT) && (pos_x < (BOARD_WIDTH)-1) && (!endgame)) pos_x++;
  if ((cmd == CMD_UP) && (pos_y > 0) && (!endgame)) pos_y--;
  if ((cmd == CMD_DOWN) && (pos_y < (BOARD_HEIGHT)-1) && (!endgame)) pos_y++;

  //  umistit symbol muze pouze hrac s kitem, ktery je na tahu
  //  nebo pokud prisel prikaz ze seriove linky (serial=0)
  if ((cmd == CMD_PLACE) && (!((serial == 0) || (det_kit == -1) || (det_kit == player_id)))) 
     cmd = CMD_NONE;

  //pokud je konec hry a stiskne se tlacitko 5, restartovat hru
  if ((cmd == CMD_PLACE) && (endgame)) 
     cmd = CMD_RESTART;

  //Umisteni symbolu
  if (cmd == CMD_PLACE) 
  { 
     ch = ticktacktoe_check(pos_x, pos_y);
     if (ch == -1) 
     { 
        //umistit symbol lze jen tehdy, je-li vybrana pozice volna
        ticktacktoe_place(pos_x, pos_y, player_id); 
        player_id = (player_id + 1) % 2; 
        move++;
        if (det_kit == -1) 
        {
           sprintf(buf, "Tah:%d Hrac:%d", move, player_id+1);
        } 
        else if (det_kit == player_id) 
        {
           sprintf(buf, "Tah:%d hrajte", move);
        } 
        else if (det_kit != player_id) 
        {
           sprintf(buf, "Tah:%d cekejte", move);
        }
        LCD_write_string(buf);

        id = (player_id & 0x1) + 2;
        //nezobrazovat, pokud neni hrac s kitem na tahu
        if ((det_kit != -1) && (det_kit != player_id))
            id = 0;

        ch = ticktacktoe_check_winner();
        if (ch == 0) 
        {
           if (det_kit == -1) 
           {
              sbuf = "Vyhral hrac: 1";
           } 
           else if (det_kit == ch) 
           {
              sbuf = "Vyhral jste";
           } 
           else 
           {
              sbuf = "Vyhral protihrac";
           }
           term_send_str_crlf(sbuf);
           LCD_write_string(sbuf);
           //zasedit symboly hrace cislo 2
           ticktacktoe_init_textures(1);
        } 
        else if (ch == 1) 
        {
           if (det_kit == -1) 
           {
              sbuf = "Vyhral hrac: 2";
           } 
           else if (det_kit == ch) 
           {
              sbuf = "Vyhral jste";
           } 
           else 
           {
              sbuf = "Vyhral protihrac";
           }
           term_send_str_crlf(sbuf);
           LCD_write_string(sbuf);
           //zasedit symboly hrace cislo 1
           ticktacktoe_init_textures(0);
        } 
        else if (ch == 2) 
        {
           sbuf = "Remiza";
           term_send_str_crlf(sbuf);
           LCD_write_string(sbuf);
        }        

        if (ch != -1) 
        {
           //Pokud je vitez nebo remiza
           //kurzor posunout za obraz
           pos_x = MAXWIDTH-BOARD_XOFFSET-1; 
           pos_y = MAXWIDTH-BOARD_YOFFSET-1;
           id = 0;
           endgame = 1;
        }

        //kdo je na rade - do horniho leveho rohu
        indexmap_write(0,0, &id);
     }
  }

  if (cmd == CMD_RESTART) 
  {
        move = 1;  
        pos_x = 0;  pos_y = 0; 
        //player_id = 0; 
        endgame = 0;

        //init 
        ticktacktoe_init_textures(-1);
        ticktacktoe_init_board();
        ticktacktoe_cursor_move(pos_x, pos_y);

        //kdo je na rade - do horniho leveho rohu
        id = (player_id & 0x1) + 2;
        //nezobrazovat, pokud neni hrac s kitem na tahu
        if ((det_kit != -1) && (det_kit != player_id))
            id = 0;
        indexmap_write(0,0, &id);

        if (det_kit == player_id) 
        {
           LCD_write_string("Jste na tahu");
           term_send_str_crlf("Jste na tahu");
        } 
        else if (det_kit != -1) 
        {
           LCD_write_string("Cekejte");
           term_send_str_crlf("Na tahu je protihrac");
        } 
        else 
        {
           sprintf(buf, "Zacina hrac:%d", player_id+1);
           term_send_str_crlf(buf);
           LCD_write_string(buf);
        }
  }

  ticktacktoe_cursor_move(pos_x, pos_y);

  //poslat prikaz na druhy kit
  if ((serial) && (cmd != CMD_NONE))
     FPGA_Buff_SendChar(48+cmd);
}

void multiplayer_init(void) 
{
  unsigned char ch;
  unsigned int tm = 0, tmm;

  term_send_crlf();
  term_send_str("Multiplayer: ");
  tm = 5000; //timeout - 5 sekund
  while (tm > 0) {
    tmm = ((rand() & 0x0F)+1)*10;
    /*
    term_send_str("Cekam A, zbyva:");
    term_send_num(tm);
    term_send_crlf();
    term_send_str("timeout:");
    term_send_num(tmm);
    term_send_crlf();
*******************************************************************************/
    while ((tm > 0) && (tmm > 0)) {
      if ((FPGA_rs232_UseBuf() != 0) && (FPGA_RS232_ReceiveChar() == 'A')) {
         FPGA_Buff_SendChar('B');
         //jsem B a detekovany 2 kity
         det_kit=1;
         tm = 0;
         break;
      }
      delay_ms(1);
      tm--;
      tmm--;
    }

    term_send_char('.');

    if (tm == 0) break;
    //term_send_str_crlf("Posilam A");
    FPGA_Buff_SendChar('A');

    while (tm > 0) {
       if (FPGA_rs232_UseBuf() != 0) {
          ch = FPGA_RS232_ReceiveChar();
          if (ch == 'B') { // jsem A a detekovany 2 kity
             //term_send_str_crlf("Jsem A a mame 2 kity");
             det_kit = 0;
             tm = 0; 
              break;
          } else if (ch == 'A') { // kolize, informovat druhou stranu a znovu
             //term_send_str_crlf("Kolize, znovu");
             FPGA_Buff_SendChar('C');
             break;
          } else if (ch == 'C') { //na druhe strane je kolize, znovu
             //term_send_str_crlf("Kolize na druhe strane, znovu");
             break;
          }
       }
         delay_ms(1);
       tm--;
    }
  }
  if (det_kit == 0) {
     term_send_str_crlf(" detekovan druhy kit (B)");
  } else if (det_kit == 1) {
     term_send_str_crlf(" detekovan druhy kit (A)");
  } else {
     term_send_str_crlf(" detekovan pouze jeden kit");
  }
}

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
   unsigned char x,y;
   char ch;

   term_send_str_crlf(" RESTART ... restart hry");
    
   //Vypis hraciho pole
   term_send_crlf();
   for (x=0; x<BOARD_WIDTH+2; x++)
       term_send_char('#');
   term_send_crlf();

   for (y=0; y<BOARD_HEIGHT; y++) {
       term_send_char('#');
       for (x=0; x<BOARD_WIDTH; x++) {
           ch = ticktacktoe_check(x,y);
           if (ch == -1) {
              term_send_char(((pos_x == x) && (pos_y == y)) ? '*' : ' ');
           } else if (ch == 0) {
              term_send_char('o');
           } else if (ch == 1) {
              term_send_char('x');
           }
       }
       term_send_char('#');
       term_send_crlf();
   }

   for (x=0; x<BOARD_WIDTH+2; x++)
       term_send_char('#');
   term_send_crlf();
}


/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd) 
{
  //Ovladani z terminalu
  if (strcmp(cmd_ucase, "8") == 0) {
     do_cmd(CMD_UP,1);
  } else if (strcmp(cmd_ucase, "4") == 0) {
     do_cmd(CMD_LEFT,1);
  } else if (strcmp(cmd_ucase, "6") == 0) {
     do_cmd(CMD_RIGHT,1);
  } else if (strcmp(cmd_ucase, "2") == 0) {
     do_cmd(CMD_DOWN,1);
  } else if (strcmp(cmd_ucase, "5") == 0) {
     do_cmd(CMD_PLACE,1);
  } else if (strcmp(cmd_ucase, "RESTART") == 0) {
     do_cmd(CMD_RESTART, 1);
  } else {
    return (CMD_UNKNOWN);
  }

  return (USER_COMMAND);
}


/*******************************************************************************
 * Obsluha preruseni z FPGA
*******************************************************************************/
void fpga_interrupt_handler(unsigned char bits)
{
  if (bits & BIT0) fpga_interrupt_rs232_rxd();
  if (bits & BIT1) fpga_interrupt_rs232_txd();
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  fpga_interrupt_init(BIT1 + BIT0);   // inicializace preruseni z fpga

  FPGA_RS232_Init();     // inicializace rs232

  LCD_init();

  LCD_write_string("Piskvorky ...");

  thermometer_init_rand();

  multiplayer_init();

  do_cmd(CMD_RESTART, 0);  
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void) 
{
  unsigned short cmd;
  unsigned char ch;
  unsigned int tm = 0;

  initialize_hardware();

  set_led_d5(1);  // Port P1, negace bitu LED0
  set_led_d6(1);  // Port P5, negace bitu LED1
 
  while (1) 
  {
    delay_ms(100);

    flip_led_d5();

    cmd = ticktacktoe_kbstate();
    if (cmd != CMD_NONE) 
    {
       flip_led_d6();
       do_cmd(cmd,1);
    }

    while (FPGA_rs232_UseBuf() != 0) 
    {
       ch =  FPGA_RS232_ReceiveChar();
       /*
       term_send_str("Prijmuto: ");
       term_send_char(ch);
       term_send_crlf();
       */
       if ((ch > 48+CMD_NONE) && (ch <= 48+CMD_RESTART)) 
       {
          do_cmd(ch - 48, 0);
       }
    }
    
    WDG_reset();                           // nulovani watchdogu
    terminal_idle();                       // obsluha terminalu
  }
}
