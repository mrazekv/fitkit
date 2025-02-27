/*******************************************************************************
   main.c : infra-red demo application
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Michal Růžek <xruzek01 AT stud.fit.vutbr.cz>

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
#include <string.h>
#include <irq/fpga_interrupt.h>
#include <infra/infra_comm.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>

//slouzi pouze pro prepinani mezi protokoly, neni podtstane pro cinnost knihoven!
#define PHILIPS 0
#define NEC 1
#define SONY12 2
#define SONY20 3

typedef struct
{
  unsigned char cmd;
  unsigned char addr;
} TRC5;


typedef struct
{
  unsigned char cmd;
  unsigned char addr;
} TSIRC12;


typedef struct
{
  unsigned char cmd;
  unsigned char addr;
  unsigned char extd;
} TSIRC20;


typedef struct
{
  unsigned char cmd;
  unsigned char addr[2];
} TNEC;


//mapovani funkci na klavesnici fitkitu: 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B | C | D | * |
const TNEC nec_key[15] = {{0x1C,{0x85,0x7A}}, {0xC1,{0x85,0x7A}}, {0x54,{0x85,0x7A}}, {0x0F,{0x85,0x7A}}, {0x15,{0x85,0x7A}},
                           {0xC9,{0x85,0x7A}}, {0x55,{0x85,0x7A}}, {0x16,{0x85,0x7A}}, {0x12,{0x85,0x7A}}, {0x95,{0x85,0x7A}},
                           {0x1A,{0x85,0x7A}}, {0x1B,{0x85,0x7A}}, {0x10,{0x85,0x7A}}, {0x11,{0x85,0x7A}}, {0x1F,{0x85,0x7A}}};
// MUTE | DVD | DTV | VCR | CD | MD | AUX | TUNER | ABCD | NIGHT | VOL+ | VOL- | PRG+ | PRG- | STDBY                           
                           
const TRC5 rc5_key[15] = {{0,0}, {1,0}, {2,0}, {3,0}, {4,0}, {5,0}, {6,0}, {7,0}, {8,0}, {9,0}, {0x10,0}, {0x11,0}, {0x20,0}, {0x21,0}, {0x0C,0}};
// 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | VOL+ | VOL- | PRG+ | PRG- | STDBY

const TSIRC12 sirc12_key[15] = {{0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0,0x11}, {0x32,0x11}, {0x38,0x11}, {0x31,0x11}, {0x30,0x11}, {0,0x11}};
// ABCD = PLAY | STOP | FORWARD | BACK


const TSIRC20 sirc20_key[15] = {{0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0,0x1A,0xFA}, {0x15,0x1A,0xFA}, {0x38,0x1A,0xFA}, {0x31,0x1A,0xFA}, {0x30,0x1A,0xFA}, {0,0x1A,0xFA}};
// A = power

//vysilany NEC ramec ulozen v txdata takto:
// [0]=ADDR_HI,  [1]=ADDR_LO   [2]=CMD
// [0]= ADDR_NEG [1]=ADDR      [2]=CMD
unsigned char txdata[3]={0,0,0};
char protokol; //typ aktualniho protokolu zde
char last_ch;  //naposledy precteny znak
char toggle_bit;
char first_press;

char pocet_opakovani;


/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf("User help:");
  term_send_str_crlf("REPEAT [1-5]                     ... pocet opakovani vyslaneho prikazu, def: 1");
  term_send_str_crlf("--= Nasledujici hodnoty jsou zadavany dvouciferne, hexadecimalne! =--");
  term_send_str_crlf("RC5 [adr] [cmd]                  ... poslani dat protokolem Philips RC5x");
  term_send_str_crlf("NEC [adr_hi/neg] [adr_lo] [cmd]  ... poslani dat protokolem NEC");
  term_send_str_crlf("SIRC12 [adr] [cmd]               ... poslani dat protokolem SONY SIRC 12bit");
  term_send_str_crlf("SIRC15 [adr] [cmd]               ... poslani dat protokolem SONY SIRC 15bit");
  term_send_str_crlf("SIRC20 [ext] [adr] [cmd]         ... poslani dat protokolem SONY SIRC 20bit");
  return;
}


/*******************************************************************************
 * Obsluha klavesnice
*******************************************************************************/
void keyboard_idle()
{
  unsigned char ch,index;
  
  //jestlize prave probiha vysilani, nebude se ramec posilat ani bufferovat:
  if (!infra_no_tx_transmit()) 
     return;
  
  ch = key_decode(read_word_keyboard_4x4());
  
  //---- detektor noveho stisku klavesy -----
  if (ch != last_ch)                                   
  {
     if (ch=='#')                     //krizek # slouzi ke zmene protokolu
     {
        protokol = (protokol+1) % 4;
        switch (protokol)
        {
           case PHILIPS:
              LCD_write_string("Proto: RC5x");
           break;
           case NEC:
              LCD_write_string("Proto: NEC");
           break;
           case SONY12:
              LCD_write_string("Proto: SIRC 12");
           break;
           case SONY20:
              LCD_write_string("Proto: SIRC 20");
           break;           
        }
     }
     if (ch!=0)
     {
        toggle_bit = (toggle_bit ? 0: 1); //zmena toggle bitu vzdy pri stisku
        first_press = 1;                  //indikace prvniho stisku teto klavesy
     }    
  }
    
  if (ch !=0 && ch!='#' )  //je nejake stiskle?
  {    
     //------- dekodovani tlacitka na index do pole ----------
     if (ch>='0' && ch<='9')
     {
        index=ch-'0';
     }
     else if (ch>='A' && ch<='D')
     {
        index=ch-'A'+10;
     }
     else if (ch=='*')
        index=14;
      
     //------- naplneni ramce ---------- 
     switch (protokol)
     {
        case PHILIPS:
           txdata[0] = toggle_bit;              //RC5 pouziva toggle bit
           txdata[1] = rc5_key[index].addr;     //5bit adresa
           txdata[2] = rc5_key[index].cmd;      //7bit prikaz     
           infra_send(IR_RC5, txdata);         
        break;
        
        case NEC:
           txdata[0] = nec_key[index].addr[0];  //16bit adresa - HIGH or NEG
           txdata[1] = nec_key[index].addr[1];  //             - LOW
           txdata[2] = nec_key[index].cmd;      //8bit prikaz
           //pokud je klavesa drzena, je vysilan opakovaci ramec:
           infra_send((first_press ? IR_NEC_NORM : IR_NEC_REPEAT), txdata);           
           first_press = 0;  
        break;
        
        case SONY12:
           txdata[0] = 00;  //subdev
           txdata[1] = sirc12_key[index].addr;  //device                    
           txdata[2] = sirc12_key[index].cmd; //command
           infra_send(IR_SIRC_12B, txdata); 
        break;
        
        case SONY20:
           txdata[0] = sirc20_key[index].extd;
           txdata[1] = sirc20_key[index].addr;  //device                    
           txdata[2] = sirc20_key[index].cmd; //command
           infra_send(IR_SIRC_20B, txdata); 
        break;        
     }          
  }
  
  //zapomatovat co bylo zmacknuto naposled
  last_ch = ch;
}


/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized() 
{
  fpga_interrupt_init(BIT0+BIT1+BIT2+BIT3+BIT4+BIT5);  //povolit masku preruseni
  infra_init(); //inicializace rutin IR komunikace
  protokol=PHILIPS;
  LCD_init();
  LCD_write_string("Proto: RC5x");  
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);//vyp kurzor  
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/

//pomocna funkce pro odeslani dat, zaradi je do bufferu
void transmit_data(char protokol)
{
  int i;
  infra_send(protokol, txdata);            //1x se posle vzdy
  for (i=1; i< pocet_opakovani; i++)
  {
     if (protokol==IR_NEC_NORM)            //NEC se neposila vickrat -> jen opakovaci prikaz
        infra_send(IR_NEC_REPEAT, txdata); 
     else
        infra_send(protokol, txdata);
  }          
}

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp6(cmd_ucase,"REPEAT") && strlen(cmd)==8)
  {
    pocet_opakovani = cmd[7]-'0';
    if (pocet_opakovani<1 || pocet_opakovani>5) pocet_opakovani=1;
    term_send_str("Vysilany prikaz se posle "); 
    term_send_char(pocet_opakovani+'0');
    term_send_str_crlf(" krat."); 
    return USER_COMMAND;
  }
  
  if (strcmp6(cmd_ucase,"SIRC12") && strlen(cmd)==12)
  {
    txdata[0] = 00;  //vzdy 0
    txdata[1] = hex2chr(cmd_ucase+7);   //device - address                   
    txdata[2] = hex2chr(cmd_ucase+10);  //command
    transmit_data(IR_SIRC_12B);
    return USER_COMMAND;
  }
  
  if (strcmp6(cmd_ucase,"SIRC15") && strlen(cmd)==12)
  {
    txdata[0] = 00;  //vzdy 0
    txdata[1] = hex2chr(cmd_ucase+7);   //device - address                   
    txdata[2] = hex2chr(cmd_ucase+10);  //command
    transmit_data(IR_SIRC_15B);
    return USER_COMMAND;
  }
  
  if (strcmp6(cmd_ucase,"SIRC20") && strlen(cmd)==15)
  {
    txdata[0] = hex2chr(cmd_ucase+7);   //extended
    txdata[1] = hex2chr(cmd_ucase+10);  //device - address                   
    txdata[2] = hex2chr(cmd_ucase+13);  //command
    transmit_data(IR_SIRC_20B);
    return USER_COMMAND;
  }  
  
  if (strcmp3(cmd_ucase,"NEC") && strlen(cmd)==12)
  {
    txdata[0] = hex2chr(cmd_ucase+4);   //address HIGH || address NEGATED!
    txdata[1] = hex2chr(cmd_ucase+7);   //address LOW                  
    txdata[2] = hex2chr(cmd_ucase+10);  //command
    transmit_data(IR_NEC_NORM);
    return USER_COMMAND;
  } 
  
  if (strcmp3(cmd_ucase,"RC5") && strlen(cmd)==9)
  {
    txdata[0] = toggle_bit;
    toggle_bit = (toggle_bit ? 0: 1);   //zmena toggle bitu
    txdata[1] = hex2chr(cmd_ucase+4);   //address
    txdata[2] = hex2chr(cmd_ucase+7);   //command                  
    transmit_data(IR_RC5);
    return USER_COMMAND;
  }         
  
  return (CMD_UNKNOWN);
}

/*******************************************************************************
 * Obsluha preruseni z FPGA
*******************************************************************************/
void fpga_interrupt_handler(unsigned char bits)
{
  if (bits & BIT0)
     infra_rx_interrupt_handler(IR_RC5);
  if (bits & BIT1)
     infra_rx_interrupt_handler(IR_NEC_NORM);
  if (bits & BIT2)
     infra_rx_interrupt_handler(IR_SIRC_12B);
  if (bits & BIT3)
     infra_rx_interrupt_handler(IR_SIRC_15B);
  if (bits & BIT4)
     infra_rx_interrupt_handler(IR_SIRC_20B);          
  if (bits & BIT5) 
     infra_tx_interrupt_handler();                 
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  unsigned int cnt = 0;
  char proto;
  unsigned char data[3];
  
  initialize_hardware();               //inicializace HW
  set_led_d6(1);                       // rozsviceni D6
  set_led_d5(1);                       // rozsviceni D5
  
  pocet_opakovani=1;                   //implicitne neopakovani vysilanych prikazu

  while (1)
  {
      while( infra_read(&proto, data) )
      {
         switch (proto)
         {
            case IR_RC5:  term_send_str("RC5x: ");
            break;
            
            case IR_NEC_NORM:  term_send_str("NEC: ");
            break;
            
            case IR_SIRC_20B: term_send_str("SIRC20: ");
            break;
            
            case IR_SIRC_15B: term_send_str("SIRC15: ");
            break;
            
            case IR_SIRC_12B: term_send_str("SIRC12: ");
            break;            
         }
         term_send_hex(data[0]); //msb - EXT / TOGGLE / ADR_HI(ADR_NEG)
         term_send_char('|');
         term_send_hex(data[1]); //    - ADR / ADR_LO
         term_send_char('|');
         term_send_hex(data[2]);   //lsb - CMD
         term_send_crlf();  
         term_send_str(">");               
      }
      
      delay_ms(10);
      cnt++;
      if (cnt > 50)
      {
         cnt = 0;
         flip_led_d6();                  // negace portu na ktere je LED
      }
      terminal_idle();  // obsluha terminalu         
      keyboard_idle();  // obsluha klavesnice        
  }         
} 
