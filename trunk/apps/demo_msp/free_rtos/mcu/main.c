/*******************************************************************************
   main: main for FreeRTOS (see http://www.freertos.org/)
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Josef Strnadel <strnadel AT stud.fit.vutbr.cz>
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
#include "FreeRTOS.h" /* see http://www.freertos.org/ */
#include "task.h"

#include <keyboard/keyboard.h>
#include <lcd/display.h>

volatile char last_ch;
unsigned char pressed = 0;

unsigned long cnt = 0;

enum eLightIntensity {lightOFF=0, lightLOW=10, lightBRIGHT=100};
enum eLightIntensity lightIntensity = lightOFF;

unsigned char charmap1[8] = {0x07, 0x08, 0x1e, 0x08, 0x00, 0x0f, 0x0e, 0x09}; 
/*  76543210            
 *  .....XXX           
 *  ....X...           
 *  ...XXXX.           
 *  ....X...           
 *  ........           
 *  ....XXXX           
 *  ....XXX.           
 *  ....X..X           
 */
 
unsigned char charmap2[8] = {0x06, 0x08, 0x08, 0x08, 0x00, 0x1f, 0x04, 0x04}; 
 /* 76543210            
 *  .....XX.           
 *  ....X...           
 *  ....X...           
 *  ....X...           
 *  ........           
 *  ...XXXXX           
 *  .....X..           
 *  .....X..           
 */
 
unsigned char charmap3[8] = {0x0c, 0x12, 0x1c, 0x1e, 0x00, 0x0c, 0x12, 0x0c}; 
 /* 76543210            
 *  ....XX..           
 *  ...X..X.           
 *  ...XXX..           
 *  ...XXXX.           
 *  ........           
 *  ....XX..           
 *  ...X..X.          
 *  ....XX..         
 */
 
unsigned char charmap4[8] = {0x0c, 0x12, 0x1c, 0x1e, 0x00, 0x0f, 0x04, 0x1e}; 
 /* 76543210            
 *  ....XX..           
 *  ...X..X.           
 *  ...XXX..           
 *  ...XXXX.          
 *  ........           
 *  ....XXXX           
 *  .....X..           
 *  ...XXXX.           
 */
 

/*******************************************************************************
 * Bodies of the RT tasks running over FreeRTOS
*******************************************************************************/

/* ---------------------
   keyboard service task
   --------------------- */
static void keyboardTask(void *param) 
{
  char ch;

  last_ch = 0;
  keyboard_init();

  for (;;) 
  {
    set_led_d6(0);

    ch = key_decode(read_word_keyboard_4x4());
    if (ch != last_ch) {
      last_ch = ch;
      if (ch != 0) {
         switch (ch) {
           case 'A':
             pressed = 1;
             set_led_d6(1);
             term_send_str_crlf("\t\tkeyboardTask: 'A' press detected");
             
             break;
           default:
             break;
         }
      }
   }
   vTaskDelay( 10 / portTICK_RATE_MS); /* delay for 10 ms (= btn-press sampling period) */ 
  }
}

/* ---------------------
   terminal service task
   --------------------- */
static void terminalTask(void *param) 
{
  for (;;) {
    terminal_idle();
    vTaskDelay( 1000 / portTICK_RATE_MS); /* delay for 1000 ms */ 
  }
}

/* -----------------
   lamp control task
   ----------------- */
static void lampTask(void *param) 
{
  pressed = 0;

  for(;;)
  {
    /*--- OFF ---*/
    lightIntensity = lightOFF;
    term_send_str_crlf("lampLight: OFF");
    term_send_str_crlf("\tlampTask: waiting for 'A' press (OFF->LOW)");
    while(!pressed); //term_send_str_crlf("waiting 4 press");
    pressed = 0;
    
    /*--- LOW ---*/
    lightIntensity = lightLOW;
    term_send_str_crlf("lampLight: LOW");
    term_send_str_crlf("\tlampTask: waiting 500 ms for 'A' press (LOW -> BRIGHT)");
    vTaskDelay( 500 / portTICK_RATE_MS); /* delay for 500 ms */ 
    term_send_str_crlf("\tlampTask: time over");

    if(pressed)
    { 
      /*--- BRIGHT ---*/
      term_send_str_crlf("\tlampTask: press detected during the time");
      pressed = 0;
      lightIntensity = lightBRIGHT;
      term_send_str_crlf("lampLight: BRIGHT");
    }
    else     term_send_str_crlf("\tlampTask: no press detected during the time");

    term_send_str_crlf("\tlampTask: waiting for 'A' press (-> OFF)");
    while(!pressed); 
    pressed = 0;
  }
}

/*******************************************************************************
 * misc functions
*******************************************************************************/
void print_user_help(void) 
{ 
}

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
}


void LCD_info(char str[LCD_CHAR_COUNT])
{
   /* display usr characters (FreeRTOS) */
   LCD_append_char('\x1');
   LCD_append_char('\x2'); 
   LCD_append_char('\x3');
   LCD_append_char('\x4');

   /* send futher data to LCD */
   LCD_send_cmd(0x0c, 0);               
   LCD_append_string(str);
}

/*******************************************************************************
 * inits preceding main() call
*******************************************************************************/
void fpga_initialized() 
{
   LCD_init();
   
   /* load usr characters (FreeRTOS) to LCD */
   LCD_load_char(1, charmap1);          
   LCD_load_char(2, charmap2);          
   LCD_load_char(3, charmap3);          
   LCD_load_char(4, charmap4);  
   
   /* welcome text */
   LCD_info(" Lamp ctrl");       
}

/*******************************************************************************
 * top level code
*******************************************************************************/
int main( void ) 
{
  /*--- HW init ---*/
  initialize_hardware();
  WDG_stop();
  P1DIR |= LED0;
  /* timer init */  
  TBCCTL0 = CCIE;           /* TIMER B interrupts enable */
  TBCCR0 = 0x0100;          /* interrupt each x ticks of clk source */
  TBCTL = TBSSEL_2 + MC_1;  /* SMCLK, continuous mode */

  term_send_crlf();

  /*--- install FreeRTOS tasks ---*/
  term_send_str_crlf("init FreeRTOS tasks...");
  xTaskCreate(terminalTask /* code */, "TERM" /* name */, 500 /* stack size */, NULL /* params */, 1 /* prio */, NULL /* handle */);
  xTaskCreate(keyboardTask, "KBD", 32, NULL, 1, NULL);
  xTaskCreate(lampTask, "LAMP", 32, NULL, 1, NULL);

  /* ------------------------------------------------
     FreeRTOS details 
     can be found at 
     http://www.freertos.org/
     ------------------------------------------------ */

  /*--- start FreeRTOS kernel ---*/
  term_send_str_crlf("starting FreeRTOS scheduler...\n");
  vTaskStartScheduler();

  return 0;
}

/*******************************************************************************
 * interrupt service routines
*******************************************************************************/
interrupt (TIMERB0_VECTOR) Timer_B (void)
{

  cnt++;

  if((cnt>=0) && (cnt<=lightIntensity)) { P1OUT=0; } /* D5 on-time */
  else { P1OUT=1; }  /* D5 off-time */
  if(cnt>=(100)) {cnt=0;}

  TBCCR0 = 0x0100;          /* interrupt each x ticks of clk source */
}
