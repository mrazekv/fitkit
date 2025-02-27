/*******************************************************************************
   main: main for 'demo_msp_signal_generator' app
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jan Krepsky <xkreps00 AT stud.fit.vutbr.cz>
              Josef Strnadel <strnadel AT stud.fit.vutbr.cz>

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
#include <lcd/display.h>
#include <stdlib.h>
#include <string.h>

#define TICKS_PER_SECOND 32768  // pocet tiku za 1s
#define SIN_SAMPLES 32    // pocet vzorku harmonickeho signalu (sinus)
#define SAW_SAMPLES 32    // pocet vzorku piloviteho signalu
#define SQUARE_SAMPLES 2  // pocet vzorku obdelnikoveho signalu
#define CONST_SAMPLES 1  // pocet vzorku konstantniho signalu
/*
 * ---------------------------------------------------------------------------- 
 */
enum SIG_ID {SIG_CONST, SIG_HARM, SIG_SAW, SIG_SQUARE} sig_type;

// pole se vzorky signalu
unsigned char arr_sin[SIN_SAMPLES] = {
   128, 153, 177, 199, 219, 234, 246, 254, 255, 254, 246, 234, 219, 199, 177,
   153, 128, 103, 79, 57, 37, 22, 10, 2, 0, 2, 10, 22, 37, 57, 79, 103
};

unsigned char arr_saw[SAW_SAMPLES] = {
  0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140,
  148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255
};
                 
unsigned char arr_square[SQUARE_SAMPLES] = {
  0, 255
};

unsigned char arr_const[CONST_SAMPLES] = {
  255
};
/*
 * ---------------------------------------------------------------------------- 
 */
unsigned char *signal_arr = arr_sin;  // ukazatel na pole vzorku zvoleneho signalu
int signal_arr_index = 0;   // index aktualniho vzorku, ktery se bude prevadet
int frequency = 100;        // frekvence signalu
unsigned char samples = SIN_SAMPLES;  // pocet vzorku zvoleneho signalu
int ticks;                  // pocet tiku, po kterych ma dojit k preruseni od casovace
int freq_max;               // maximalni frekvence zvoleneho signalu, kterou lze generovat
unsigned char y_scale = 100;          // zmena meritka osy Y na dany pocet % (0...100) 

unsigned char str_tmp[MAX_COMMAND_LEN+1];
/*
 * ---------------------------------------------------------------------------- 
 */
void get_data(unsigned char *str, int from);
void set_ticks(void);
void set_freq_max(void);

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf("Napoveda pro generator signalu:");
  term_send_str_crlf(" SIG harm ..... harmonicky signal");
  term_send_str_crlf(" SIG saw ...... pilovity signal");
  term_send_str_crlf(" SIG square ... obdelnikovy signal");
  term_send_str_crlf(" SIG const ... konstantni signal");
  term_send_str_crlf("");
  term_send_str_crlf(" SCALE x ... zmena amplitudy napeti na x = 0 ... 100 [%]");
  term_send_str_crlf(" FREQ f ....... zmena frekvence [Hz]");
  term_send_str_crlf("");
  term_send_str_crlf("Napeti je meritelne mezi vyvodem 31 (konektoru JP9) a zemi FITkitu");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *UString, char *String)
{	 
	if (strcmp4(UString, "SIG "))  // zmena typu signalu
	{     
      get_data(String, 4);  // do str_tmp ulozi UString od 4. znaku dal
		unsigned char *msg;
		
	  if (strcmp3(str_tmp, "harm"))  // harmonicky
	  {
	    sig_type = SIG_HARM;
	    signal_arr = arr_sin;
	    signal_arr_index = 0;
	    samples = SIN_SAMPLES;
   	  msg = "Generuju harmonicky signal...";
	  }
	  else if (strcmp3(str_tmp, "saw"))  // pilka
	  {
	    sig_type = SIG_SAW;
	    signal_arr = arr_saw;
	    signal_arr_index = 0;
	    samples = SAW_SAMPLES;
	    msg = "Generuju pilovity signal...";
	  }
	  else if (strcmp6(str_tmp, "square"))  // obdelnik
	  {
	    sig_type = SIG_SQUARE;
	    signal_arr = arr_square;
	    signal_arr_index = 0;
	    samples = SQUARE_SAMPLES;
	    msg = "Generuju obdelnikovy signal...";
	  }
	  else if (strcmp6(str_tmp, "const"))  // konstanta
	  {
	    sig_type = SIG_CONST;
	    signal_arr = arr_const;
	    signal_arr_index = 0;
	    samples = CONST_SAMPLES;
	    msg = "Generuju konstantni signal...";
	  }
	  else
	  {
	    term_send_str_crlf("Neznamy signal.");
	    return USER_COMMAND;
	  }
	  
    set_freq_max();
	  
	  // zkontrolujeme, jestli aktualni frekvence neni vyssi, nez maximalni
	  // povolena pro vybrany druh signalu
	  if (frequency <= freq_max)
	  {
	    set_ticks();
    }
    else
	  {
	    frequency = freq_max;
	    set_ticks();
	    
	    itoa(freq_max, str_tmp, 10);
	    
	    term_send_str("Frekvence snizena na ");
      term_send_str(str_tmp);
      term_send_str_crlf(" Hz.");
	  }
	  
	  term_send_str_crlf(msg);
  }
  else if (strcmp6(UString, "SCALE "))  // zmena meritka osy Y
	{     
     get_data(String, 6);  // do str_tmp ulozi UString od 6. znaku dal  
	  y_scale = atoi(str_tmp);

    if(y_scale < 0) 
    {
      term_send_str_crlf("Chybne meritko (< 0) zmeneno na hodnotu 0 %.");
      y_scale = 0;
    }
    else if(y_scale > 100)
    {
      term_send_str_crlf("Chybne meritko (> 100) zmeneno na hodnotu 100 %.");
      y_scale = 100;
    }
    else
    {
      term_send_str("Amplituda nastavena na ");
      term_send_str(str_tmp);
      term_send_str_crlf(" %.");
    }
  }
  else if (strcmp5(UString, "FREQ "))  // zmena frekvence
	{     
    get_data(String, 5);  // do str_tmp ulozi UString od 5. znaku dal  
	  int freq_tmp = atoi(str_tmp); 
	  
	  if (freq_tmp > 0 && freq_tmp <= freq_max)
	  {
	    frequency = freq_tmp;
	    set_ticks();
	    
      term_send_str("Frekvence zmenena na ");
      term_send_str(str_tmp);
      term_send_str_crlf(" Hz.");
    }
    else if (freq_tmp > freq_max)
    {
      itoa(freq_max, str_tmp, 10);
      
      term_send_str("Chybna frekvence. Maximalni hodnota pro tento signal je ");
      term_send_str(str_tmp);
      term_send_str_crlf(" Hz.");
    }
    else
    {
      term_send_str_crlf("Chybna frekvence.");
    }

  }
	else { return (CMD_UNKNOWN); }

  return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
   LCD_init();
   LCD_append_string("Signal-generator");
   LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);  // vypni kurzor

  term_send_str_crlf("Napetovy prubeh, ktery chcete generovat, nastavte pomoci prikazu (viz help).");
  term_send_str_crlf("Napeti je meritelne mezi vyvodem 31 (konektoru JP9) a zemi FITkitu");
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  initialize_hardware();
  WDTCTL = WDTPW + WDTHOLD;	 // zastav watchdog

  // nastaveni maximalni frekvence signalu a poctu tiku do preruseni
  set_freq_max();
  set_ticks();
  
  // nastaveni casovace
	CCTL0 = CCIE;  // povol preruseni pro casovac (rezim vystupni komparace) 
  CCR0 = ticks; // nastav po kolika ticich ma dojit k preruseni
  TACTL = TASSEL_1 + MC_2;  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

  // nastaveni DAC
  ADC12CTL0 |= 0x0020;    // nastaveni referencniho napeti z ADC na 1,5 V
  DAC12_0CTL |= 0x1060;   // nastaveni kontrolniho registru DAC (8-bitovy rezim, medium speed)
  DAC12_0CTL |= 0x100;    // vystupni napeti nasobit 1x
  DAC12_0DAT = 0;         // vynulovani vystupni hodnoty DAC

  // hlavni smycka
  while (1)
	{
    terminal_idle();      // obsluha terminalu
  }
}

/*******************************************************************************
 * Obsluha preruseni casovace timer A0
*******************************************************************************/
interrupt (TIMERA0_VECTOR) Timer_A (void)
{  
  if (++signal_arr_index >= samples){ signal_arr_index = 0; }
  unsigned int smpl = (signal_arr[signal_arr_index]*y_scale)/100;
  DAC12_0DAT = smpl;  // nahrani dalsiho vzorku pro prevod
  
  CCR0 += ticks;  // nastav po kolika ticich ma dojit k dalsimu preruseni
}

/*******************************************************************************
 * Funkce ulozi do str_tmp retezec str od znaku from do konce
*******************************************************************************/
void get_data(unsigned char *str, int from)
{
	int i, j = 0;
	for (i = from; i < strlen(str); i++){	str_tmp[j++] = str[i]; }
	str_tmp[j] = 0;
}

/*******************************************************************************
 * Funkce nastavi pocet tiku (do promenne ticks), za kolik dojde k preruseni
 * od casovace na zaklade zadane frekvence a funkce (jejich vzorku) 
*******************************************************************************/
void set_ticks(void)
{
  ticks = TICKS_PER_SECOND / frequency / samples;
}

/*******************************************************************************
 * Funkce nastavi do promenne freq_max maximalni hodnotu frekvence pro zvoleny signal
*******************************************************************************/
void set_freq_max(void)
{
  // maximalni frekvence je odvozena od poctu vzorku zvoleneho signalu
  freq_max = TICKS_PER_SECOND / samples / 2;
}

