/*******************************************************************************
   ps2_keyb.c: PS2 KEYBOARD
   Copyright (C) 2008 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Pavel Bartos <xbarto41 AT stud.fit.vutbr.cz>

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
//#define DEBUG_PS2


#include <msp430x16x.h>
#include <fitkitlib.h>
#include "ps2_keyb.h"


#define _LED_SCROLL 1
#define _LED_NUM 2
#define _LED_CAPS 4

//stavy PS/2 klavesnice
enum _ePS2State {_PS2_INIT_1, _PS2_INIT_2, _PS2_INIT_3, _PS2_INIT_4, _PS2_INIT_5, _PS2_SET_LED, _PS2_READY, _PS2_SEQUENCE, _PS2_ERROR};

enum _ePS2State _PS2state = _PS2_INIT_1; //aktualni stav
enum _ePS2State _PS2next_state = _PS2_INIT_1; //nasledujici stav
unsigned char _ps2_leds = 0; //LED diody
unsigned char _state_keys = 0; //stavove klavesy (shift, ctrl...)
unsigned char _last_send;  //posledni poslany byte
unsigned char _init_in_progress;
void (*_callback)(unsigned char);




enum eFuncKeys {
KEY_L_CTRL=128, KEY_NUM=129, KEY_CAPS=131, KEY_SCROLL=132,  KEY_L_ALT=135, 
KEY_L_SHIFT=143, KEY_R_CTRL=159, KEY_F1=161, KEY_F2=162, KEY_F3=163, KEY_F4=164, KEY_F5=165, KEY_F6=166, 
KEY_F7=167, KEY_F8=168, KEY_F9=169, KEY_F10=170, KEY_F11=171, KEY_F12=172, KEY_R_ALT=191, KEY_KP_MINUS=192, 
KEY_KP_PLUS=193, KEY_KP_0=194, KEY_KP_1=195, KEY_KP_2=196, KEY_KP_3=197, 
KEY_KP_4=198, KEY_KP_5=199, KEY_KP_6=200, KEY_KP_7=201, KEY_KP_8=202, 
KEY_KP_9=203, KEY_HOME=204, KEY_UP=205, KEY_PG_UP=206, KEY_LEFT=207, KEY_RIGHT=208, KEY_END=209, 
KEY_DOWN=210, KEY_PG_DN=211, KEY_INSERT=212, KEY_DELETE=213, KEY_L_WIN=214, KEY_R_WIN=215, KEY_APPS=216,
KEY_ESC=217, KEY_R_SHIFT=255 
};



const unsigned char virtual_codes_set1[128] = {
	0xFF, KEY_ESC, '1', '2', '3', '4', '5', '6',
 '7', '8', '9', '0', '-', '=', '\b', '\t',
 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
 'o', 'p', '[', ']', '\n', KEY_L_CTRL, 'a', 's',
 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
 '\'', '`', KEY_L_SHIFT, '\\', 'z', 'x', 'c', 'v',
 'b', 'n', 'm', ',', '.', '/', KEY_R_SHIFT, '*',
 KEY_L_ALT, ' ', KEY_CAPS, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
 KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUM, KEY_SCROLL, KEY_KP_7,
 KEY_KP_8, KEY_KP_9, KEY_KP_MINUS, KEY_KP_4, KEY_KP_5, KEY_KP_6, KEY_KP_PLUS, KEY_KP_1,
 KEY_KP_2, KEY_KP_3, KEY_KP_0, '.', 0xFF, 0xFF, 0xFF, KEY_F11,
 KEY_F12, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

const unsigned char virtual_codes_set1_shift[128] = {
	0xFF, KEY_ESC, '!', '@', '#', '$', '%', '^',
 '&', '*', '(', ')', '_', '+', '\b', '\t',
 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
 'O', 'P', '{', '}', '\n', KEY_L_CTRL, 'A', 'S',
 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
 '\"', '~', KEY_L_SHIFT, '|', 'Z', 'X', 'C', 'V',
 'B', 'N', 'M', '<', '>', '?', KEY_R_SHIFT, '*',
 KEY_L_ALT, ' ', KEY_CAPS, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
 KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUM, KEY_SCROLL, KEY_HOME,
 KEY_UP, KEY_PG_UP, KEY_KP_MINUS, KEY_LEFT, 0xFF, KEY_RIGHT, KEY_KP_PLUS, KEY_END,
 KEY_DOWN, KEY_PG_DN, KEY_INSERT, KEY_DELETE, 0xFF, 0xFF, 0xFF, KEY_F11,
 KEY_F12, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


const unsigned char virtual_codes_set1_E0[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, '\n', KEY_R_CTRL, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, '/', 0xFF, 0xFF,
 KEY_R_ALT, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, KEY_HOME,
 KEY_UP, KEY_PG_UP, 0xFF, KEY_LEFT, 0xFF, KEY_RIGHT, 0xFF, KEY_END,
 KEY_DOWN, KEY_PG_DN, KEY_INSERT, KEY_DELETE, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, KEY_L_WIN, KEY_R_WIN, KEY_APPS, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};



/**
 \brief Inicializace klavesnice
 */
void keyboard_ps2_init(void (*callback)(unsigned char))
{
  _init_in_progress = 1;

  _callback=callback;
  _PS2state = _PS2_INIT_1;
  keyboard_ps2_FM();

}

/**
 \brief Odesle byte do klavesnice
 */
void keyboard_ps2_send_char(unsigned char data)
{
  _last_send = data;
  FPGA_SPI_RW(AN_D8,SPI_FPGA_ENABLE_WRITE, BASE_ADDR_KEYBOARD, data);
  #ifdef DEBUG_PS2
  term_send_str("***DEBUG: PS2 SEND = ");
  term_send_hex(data);
  term_send_crlf();
  #endif 
}

/**
 \brief Automat klavesnice
 */
void keyboard_ps2_FM()
{
  unsigned char data = FPGA_SPI_RW(AN_D8,SPI_FPGA_ENABLE_READ, BASE_ADDR_KEYBOARD, 0); //precte data z klavesnice
  unsigned char data2; 
    #ifdef DEBUG_PS2
    term_send_str("***DEBUG: PS2 SCAN = ");
    term_send_hex(data);
    term_send_str("   STATE = ");
    term_send_hex(_PS2state);
    term_send_crlf();
    #endif
  
  if (data == 0xFE) //resend
  {
    keyboard_ps2_send_char(_last_send);
    return;
  }
  
  switch (_PS2state)
  {
    case _PS2_INIT_1:
      keyboard_ps2_send_char(0xFF); //reset keyboard
      _PS2next_state = _PS2_INIT_2;
      break;
    case _PS2_INIT_2:
      if (data == 0xFA) //ACK
        _PS2next_state = _PS2_INIT_3;
      else
        _PS2next_state = _PS2_ERROR;
      break;
    case _PS2_INIT_3:
      if (data == 0xAA /*BAT success*/)
      {
        _PS2next_state = _PS2_INIT_4;
        keyboard_ps2_send_char(0xF0); //bude se nastavovat sada SCAN kodu
      }
      else
        _PS2next_state = _PS2_ERROR;    
      break;
    case _PS2_INIT_4:
      if (data == 0xFA /*ACK*/)
      {
        keyboard_ps2_send_char(0x01); //sada 1
        _PS2next_state = _PS2_INIT_5;
      }
      else
        _PS2next_state = _PS2_ERROR;
      break;
    case _PS2_INIT_5:
      if (data == 0xFA) //ACK
      {
        _ps2_leds |= _LED_NUM;
        _state_keys |= KEY_NUM-127;
        keyboard_ps2_send_char(0xED); //nastavi NUM LOCK LED
        _PS2next_state = _PS2_SET_LED;
      }
      else
        _PS2next_state = _PS2_ERROR;
      break;
    case _PS2_SET_LED:
      if (data == 0xFA) //ACK
      {
        keyboard_ps2_send_char(_ps2_leds);
        _PS2next_state = _PS2_READY;
        _init_in_progress = 0;
      }
      else
        _PS2next_state = _PS2_ERROR;
      break;
    case _PS2_READY: //zakladni stav
    {
      if (data == 0xFF) break;
      if (data == 0xE0)
        _PS2next_state = _PS2_SEQUENCE; //zaactek sekvence
      else
      {
        data2 = virtual_codes_set1[data & 0x7F];  //prevod scan kodu na znak
        
        //numericka klavesnice
        if (data2 >= KEY_KP_0 && data2 <= KEY_KP_9)
        {
          if (!(_state_keys & (KEY_NUM-127) && !(_state_keys & (KEY_L_SHIFT-127) || _state_keys & (KEY_R_SHIFT-127))))
          { //numlock_off  OR (numlock_on AND shift)
            data2 = virtual_codes_set1_shift[data & 0x7F]; //prevod klaves numericke klavesnice na funkcni klavesy (napr. 7->Home)
          }
          else
          {
            data2 -= KEY_KP_0;
            data2 += '0';
          }
        }
        
        //CAPS LOCK
        else if (_state_keys & (KEY_CAPS-127)) 
        { //CAPS LOCK on
          if ((data2 >= 'a' && data2 <= 'z' && !(_state_keys & (KEY_L_SHIFT-127) || _state_keys & (KEY_R_SHIFT-127)))
           || ((data2 < 'a' || data2 > 'z') && (_state_keys & (KEY_L_SHIFT-127) || _state_keys & (KEY_R_SHIFT-127))))
          { //letter and not_shift  OR  not_letter and shift
            data2 = virtual_codes_set1_shift[data & 0x7F]; //mala pismena prevede na velka
          }
        }
        
        //SHIFT
        else
        {
          if (_state_keys & (KEY_L_SHIFT-127) || _state_keys & (KEY_R_SHIFT-127))
          {
            data2 = virtual_codes_set1_shift[data & 0x7F];
          }
        }
          
        switch (data2)
        {
          //obsluha LED klavesnice
          case KEY_NUM:
          case KEY_CAPS:
            if (!(data & 0x80))
            {
              //_ps2_leds ^= data2 - 127;
              _state_keys ^= data2 - 127;
              ps2_set_leds(ps2_get_leds() ^ (data2-127));
              return;
            }
            break; 
          case KEY_L_CTRL:
          case KEY_R_CTRL:
          case KEY_L_ALT:
          case KEY_R_ALT:
          case KEY_L_SHIFT:
          case KEY_R_SHIFT:
            if (data & 0x80) //released state key
              _state_keys &= ~(data2 - 127);
            else //pressed state key
              _state_keys |= data2 - 127;
            break;
          default:
            if (data & 0x80) break;
              #ifdef DEBUG_PS2
              term_send_str("***DEBUG: PS2 SCAN: ");
              term_send_hex(data);
              term_send_str(" KEY: ");
              term_send_char(data2);
              term_send_crlf();
              #endif          
            _callback(data2); //dalsi zpracovani znaku
            break;
        }
      }
      break;
    }
    case _PS2_SEQUENCE:
      data2 = virtual_codes_set1_E0[data & 0x7F]; //prevod scan kodu
      if (data2 == 0xFF) _PS2next_state = _PS2_ERROR;
      switch (data2)
      {
          case KEY_L_CTRL:
          case KEY_R_CTRL:
          case KEY_L_ALT:
          case KEY_R_ALT:
          case KEY_L_SHIFT:
          case KEY_R_SHIFT:
          if (data & 0x80) //released state key
            _state_keys &= ~(data2 - 127);
          else //pressed state key
            _state_keys |= data2 - 127;
          break;
        default:
          if (data & 0x80) break;
            #ifdef DEBUG_PS2
            term_send_str("***DEBUG: PS2 SEQ SCAN: ");
            term_send_hex(data);
            term_send_str(" KEY: ");
            term_send_char(data2);
            term_send_crlf();
            #endif          
          _callback(data2); //dalsi zpracovani
          break;
      }
      _PS2next_state = _PS2_READY;      
      break;
    case _PS2_ERROR:
      break;
    
    default:
      _PS2next_state = _PS2_READY;
      break;
  }
  if (_PS2next_state == _PS2_ERROR) //v pripade chyby vypise aktualni stav
  {
    term_send_str("PS/2 keyboard error. Act state = ");
    term_send_num(_PS2state);
	  term_send_crlf();
  }
  #ifdef DEBUG_PS2
    term_send_str("***DEBUG: PS2 state keys = ");
	  term_send_hex(_state_keys);
    term_send_crlf();
  #endif

  _PS2state = _PS2next_state;
  return;
}

/**
 \brief Nastavi LED klavesnice
 */
void ps2_set_leds(unsigned char leds)
{
  _ps2_leds = leds;
  _PS2state = _PS2next_state = _PS2_SET_LED; //nastavi stav automatu tak, aby ocekaval odpoved ACK a potom presel do stavu READY
  keyboard_ps2_send_char(0xED);
}

/**
 \brief Vrati stav LED klavesnice
 */
unsigned char ps2_get_leds()
{
  return _ps2_leds;
}
