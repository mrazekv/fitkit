/*
* uC/OS-II pro platformu FITkit
* Autori: Karel Koranda     xkoran01@stud.fit.vutbr.cz
*         Vladimir Bruzek   xbruze01@stud.fit.vutbr.cz
*         Tomas Jilek       xjilek02@stud.fit.vutbr.cz  
* Datum: 26-11-2010
*
* Ukazkova aplikace bezici nad uC/OS-II
* Aplikace pro FITKit 2.0: 
*   Aplikaci implementovali vyse uvedeni autori.
*   OS zpracovava tri ulohy.
*     Prvni uloha - Obsluhuje klavesnici na FITKitu. Pokud je stisknuto tlacitko
*     'A', je rozsvicena cervena dioda D6. Pokud je stisknuto znovu, je zhasnuta.
*     Pokud je stisknuto tlacitko 'B', je rozsvicena zelena dioda D5, pokud je
*     stisknuto znovu, je zhasnuta. Pri stisknuti klaves 'A', 'B', 'C', 'D', '*'
*     je nastaven priznak o jejich stisku. Po stisku kazde klavesy je na zaklade 
*     priznaku odeslana na terminal zprava o provedene akci.
*     
*     Druha uloha - Pracuje s displayem. Pokud je nastaven priznak stisku klavesy
*     'C', je displej povazovan za vypnuty. Pokud je nastaven priznak klavesy d,
*     jsou na displej vypisovany udaje ziskane od statisticke funkce. Pokud je
*     nastaven priznak odpovidajici klavese '*', pak je rozeni displeje ponechano
*     na treti uloze. 
*     
*     Treti uloha je zatezova uloha. Pokud je nastaven priznak '*', kazdych 50ms
*     (tim je generovana vyssi zatez) vypisuje na displej jeji nazev a aktualni 
*     zatizeni mikroprocesoru (udaj ze statisticke funkce).  
*
*     Statisticka uloha je na FITKitu 2.0 funkcni. Je inicializovana v ramci prvni
*     ulohy, pokud je v souboru os_cfg.h nastavena konstanta OS_TASK_STAT_EN na 1
*     (implicitne tomu tak je). Pokud je nastavena na 0, treti, zatezova uloha
*     vypisuje na displej pouze svuj nazev. Vypisovani udaju o zatizeni v ramci
*     druhe ulohy taktez neni povoleno (namisto toho je vypisovano STATS DISABLED).
*
*     OS ma vytvorenu jeste jednu, zakladni ulohu, ktera inicializuje vsechny
*     ostatni a statistickou funkci. Proc tomu tak je, je zduvodneno
*     v dokumentaci, resp. v dalsi literature.
*
* Aplikace pro FITKit 1.2:
*   Tuto aplikaci implementovali:
*     Jiri Novotnak
*     Tomas Novotny
*   OS zpracovava dve ulohy.
*     Prvni uloha blika zelenou diodou kazdych 100ms.
*     Druha uloha blika cervenou diodou kazdou sekundu na 100ms.
*/


#include "includes.h"
#include <signal.h>

#include <fitkitlib.h>
#include <lcd/display.h>
#include <keyboard/keyboard.h>

#if FITKIT_VERSION == 1
#include <msp430x16x.h>
#elif FITKIT_VERSION == 2
#include <msp430x261x.h>
#include <stdio.h>
#else
#error Bad and unsupported version of FITKit!
#endif

// Nastaveni hodin pro FITkit
#define ACLK            BIT6
#define SMCLK           BIT5

#define OS_TASK_DEF_STK_SIZE 128
#define TASK_STK_SIZE OS_TASK_DEF_STK_SIZE    // Velikost zasobniku pro kazdy ukol (bajty)

// zasobniky urcene pro jednotlive ulohy
#if FITKIT_VERSION == 1

OS_STK TaskGreenLEDStk[TASK_STK_SIZE];  // uloha blikani zelene diody
OS_STK TaskRedLEDStk[TASK_STK_SIZE];    // uloha blikani cervene diody

#elif FITKIT_VERSION == 2

OS_STK TaskStartStk[TASK_STK_SIZE];     // spousteci uloha
OS_STK TaskKeyStk[TASK_STK_SIZE];       // uloha obsluhujici klavesnici
OS_STK TaskDisplayStk[TASK_STK_SIZE];   // uloha obsluhujici displej
OS_STK TaskLoadStk[TASK_STK_SIZE];      // zatezova uloha

#endif

// Funkce nutne pro definovani kvuli includovani knihovny libfitkit
void fpga_initialized() {}

void print_user_help() {}

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return(CMD_UNKNOWN);
}

/*
* Definice uloh a HW inicializace pro FITKit verze 1.2
*/
#if FITKIT_VERSION == 1

/*
 * Uloha blikajici se zelenou LED diodou (D5)
 * Kazdych 100ms prepina stav diody
 */
void green_task(void *data) {
   P1OUT &= ~BIT0;
   // Pro statistickou ulohu je potreba povolit OS_TASK_STAT_EN v os_cpu.h v adresari s portem
   // XXX Zapnuti statisticke ulohy vsak zpusobi nefunkcnost OS (po provedeni je rozbity TCB)!
#if OS_TASK_STAT_EN
  #if FITKIT_VERSION == 2
   OSStatInit();
  #else
    #error STAT Task is only functional on FITKit 2.0!
  #endif
#endif

   for(;;) {
      OSTimeDlyHMSM(0,0,0,100);
      P1OUT ^= BIT0;
   }
}

/*
 * Uloha blikajici s cervenou LED diodou (D6)
 * Kazdou sekundu blikne na 100ms
 */
void red_task(void *data) {
   P5OUT |= BIT0;
   for(;;) {
      OSTimeDlyHMSM(0,0,0,900);
      P5OUT &= ~BIT7;
      OSTimeDlyHMSM(0,0,0,100);
      P5OUT |= BIT7;
   }
}

/*
 * Inicializuje hardware - hodinovy system
 * Inspirovano kodem z knihovny libfitkit
 */
void my_initialize_hardware() {

    WDTCTL = WDTPW + WDTHOLD;             /// Stop WDT
    IE1 &= ~WDTIE;
     
    /// pocatecni nastaveni hodin
    BCSCTL1 = RSEL2 + RSEL1 + RSEL0;      /// nizka frekvence na LFXT1, vys. fr. na LFXT2
                                          /// DCO na 3.0MHz, ACLK na 32.768kHz
    /// nabeh krystalovych oscilatoru
    _BIC_SR(OSCOFF);                      /// nulovani OscOFF, povoleni LFXT1
    do {
      IFG1 &= ~OFIFG;                     /// nulovani OFIFG
       volatile unsigned int i = 0;
       while (1) { // zpozdeni
          if (i == 1000)
             break;
          i++;
       }
    } while ((IFG1 & OFIFG) != 0);        /// test nabehnuti oscilatoru LFXT1 a LFXT2

    BCSCTL2 = SELM_2 + SELS;              /// SMCLK = LFXT2 = 7.3728 MHz, MCLK = LFXT2 = 7.3728 MHz

    /// INICIALIZACE vystupu hodin pro FPGA
    P5DIR |= SMCLK + ACLK;         /// nastavi piny jako vystupni
    P5SEL |= SMCLK;                ///< pripoji SMCLK na pin
}

/*
* Definice uloh pro FITKit verze 2.0
*/
#elif FITKIT_VERSION == 2

volatile char last_ch;
unsigned char c_pressed = 0;            // priznak stisku klavesy 'C'
unsigned char d_pressed = 0;            // priznak stisku klavesy 'D'
unsigned char load_pressed = 0;         // priznak stisku klavesy '*'
unsigned char previous_c_pressed = 0;   // priznak zpracovaneho stisku klavesy 'C'

unsigned char light_red = 1;            // priznak stisku klavesy 'A' - dioda D6
unsigned char light_green = 1;          // priznak stisku klavesy 'B' - dioda D5

/*
* Uloha pro zpracovani klavesnice
* 
* Na zaklade vstupu klavesnice rozhoduje o nastaveni prislusnych priznaku.
* Pokud jsou stisknuty klavesy 'A' a 'B', rozsvecuje/zhasina prislusne diody.
* Po stisku vsech klaves vypisuje do terminalu FITKitu udaje o tom, co dana
* klavesa aktivovala/deaktivovala.
*/
void keyboardTask(void *param) 
{
  param = param;
  char ch;

  last_ch = 0;
  keyboard_init(); // inicializace klavesnice

  for (;;) 
  {
    ch = key_decode(read_word_keyboard_4x4()); // dekodovani znaku z klavesnice
    if (ch != last_ch) {
      last_ch = ch;
      if (ch != 0) {
         switch (ch) {
           case 'A': // stisk klavesy 'A'
             set_led_d6(light_red); // nastaveni rozsviceni/zhasnuti diody
             if (light_red == 1)
             { // dioda byla rozsvicena
                term_send_str_crlf("'A': Turn on red light D6.");
             } else { // dioda byla zhasnuta
                term_send_str_crlf("'A': Turn off red light D6.");
             }
             // nastaveni pristiho stavu diody
             light_red = (light_red == 0 ? 1 : 0); 
             break;
          case 'B': // stisk klavesy 'B'
             set_led_d5(light_green); // rozsviceni/zhasnuti diody
             if (light_green == 1)
             { // dioda je rozsvicena
                term_send_str_crlf("'B': Turn on green light D5.");
             } else { // dioda je zhasnuta
                term_send_str_crlf("'B': Turn off green light D5.");
             }
             // nastaveni pristiho stavu diody
             light_green = (light_green == 0 ? 1 : 0);
             break;
           case 'C': // stisk klavesy 'C'
             c_pressed = (c_pressed == 0 ? 1 : 0); // nastaveni priznaku
             if (c_pressed) { // stisknuta klavesa -> vypnuty displej
                term_send_str_crlf("'C': Display OFF");
             } else { // spusteny displej
                term_send_str_crlf("'C': Display ON");
             }
             break;
           case 'D':
             d_pressed = (d_pressed == 0 ? 1 : 0); // nastaveni priznaku
             if (d_pressed) { // stisknuta klavesa -> vypis statistik
                term_send_str_crlf("'D': CPU Usage Monitoring ON");
             } else { // vypnuty vypis statistik
                term_send_str_crlf("'D': CPU Usage Monitoring OFF");
             }
             break;
           case '*':
             load_pressed = (load_pressed == 0 ? 1 : 0); // nastaveni priznaku
             if (load_pressed) { // stisknuta klavesa -> zatezova uloha
                term_send_str_crlf("'*': Load task ON");
             } else { // vypnuta zatezova uloha
                term_send_str_crlf("'*': Load task OFF");
             }
             break;
           default:
             break;
         }
      }
   }
   OSTimeDlyHMSM(0,0,0,100); // zpozdeni 100ms
  }
}

/*
* Zatezova uloha
* 
* Uloha vypisuje kazdych 50ms svuj nazev na displej, cimz generuje podstatnou
* zatez. Touto zatezi chceme demonstrovat funkcnost statisticke ulohy.
* Pokud je OS_TASK_STAT_EN v souboru os_cfg.h nastavena na 1 (tedy statisticka
* uloha je generovana), vypisuje tato uloha na displej take aktualni zatez.
* 
* Uloha je zapnuta pouze v pripade, ze byla spustena klavesou '*'.
*/
void loadTask(void *param)
{
  for (;;)
  {
    while (load_pressed) // dokud je nastaven priznak stisknute '*'
    {
      if (!c_pressed) { // pokud neni vypnuty displej
        if (previous_c_pressed) previous_c_pressed = 0; // pokud je treba, prehod stav displeje
#if OS_TASK_STAT_EN // je generovana statisticka funkce
        char result[32];
        sprintf(result, "!!!LOAD TASK!!! CPU %d%c", OSCPUUsage, '\%');
        LCD_write_string(result);
#else // neni generovana statisticka funkce
        LCD_write_string("!!!LOAD TASK!!! STATS DISABLED");
#endif
      }
      OSTimeDlyHMSM(0,0,0,50); // zpozdeni 50ms
    }
    OSTimeDlyHMSM(0,0,1,0); // zpozdeni 1s
  }
}

/*
* Uloha obsluhujici displej
* 
* Uloha na zaklade nastavenych priznaku vypisuje na displej ruzne zpravy.
* Stavy:
*   Neni nastaven priznak 'C', 'D', '*':
*     Na displej jsou vypisovany udaje o rozsvicenych/zhasnutych diodach.
*   Je nastaven priznak 'C':
*     Displej je vypnuty (nic na nej neni vypisovano)
*   Neni nastaven priznak 'C', '*', je nastaven 'D':
*     Na displej je vypisovan udaj ze statisticke funkce ve formatu "CPU X%",
*     tentyz udaj je odesilan na terminal FITKitu.
*   Neni nastaven priznak 'C' a 'D', je nastaven '*':
*     Obsluha displeje je nechana na zatezove uloze loadTask.
*   Neni nastaven priznak 'C', je nastaven 'D' a '*':
*     Obsluha displeje je nechana na zatezove uloze loadTask. Na terminal
*     FITKitu jsou odesilany informace o zatezi ve formatu "CPU X%".
*/
void displayTask(void *param)
{
   LCD_init(); // inicializace LCD
   
   for (;;)
   {
   
      if (d_pressed) { // nastaven priznak 'D'
#if OS_TASK_STAT_EN // je generovana statisticka uloha
        char result[20];
        sprintf (result, "CPU %d%c", OSCPUUsage, '\%'); // format vystupu stat.funkce
        if (!c_pressed && !load_pressed) { // pokud neni priznak 'C' a '*'
          if (previous_c_pressed) previous_c_pressed = 0;
          LCD_write_string(result); // vypis na displej
        }
        term_send_str_crlf(result); // odeslani dat na terminal
#else
        if (!c_pressed && !load_pressed) {
          if (previous_c_pressed) previous_c_pressed = 0;
          LCD_write_string("STATS DISABLED");
        }
        term_send_str_crlf("STATS DISABLED");
#endif
        OSTimeDlyHMSM(0,0,1,0);
      } else { // neni nastaven 'D'
        if (!c_pressed && !load_pressed) { // neni nastaven 'C' ani '*'
          if (previous_c_pressed) previous_c_pressed = 0;
          // vypis udaju o diodach
          if (light_red == 0 && light_green == 0) {
              LCD_write_string("D6:ON D5:ON");
            } else if (light_red == 1 && light_green == 0) {
              LCD_write_string("D6:OFF D5:ON");
            } else if (light_red == 1 && light_green == 1) {
              LCD_write_string("D6:OFF D5:OFF");
            } else {
              LCD_write_string("D6:ON D5:OFF");
            }
            OSTimeDlyHMSM(0,0,0,500); // zpozdeni 0.5s
        }
        if (load_pressed) // je nastaven priznak '*' zatezove ulohy
        { // tato uloha nebude nyni obsluhovat displej
          OSTimeDlyHMSM(0,0,0,500); // zpozdeni 0.5s
        }
      }
      if (c_pressed)
      { // je nastaven priznak vypnuteho displeje (klavesa 'C')
        if (previous_c_pressed == 0) // pokud jsme nastaveni priznaku identifikovali
        { // poprve
          LCD_write_string(" "); // cisteni displeje
          previous_c_pressed = 1; // priste uz cistit nebudeme, setrime
        }
        OSTimeDlyHMSM(0,0,0,500); // zpozdeni 500ms
      }
   } 
}

/*
* Startovaci uloha
* 
* Vytvari v OS dalsi ulohy a inicializuje statistickou ulohu, pokud je to
* nastaveno v souboru os_cfg.h konstantou OS_TASK_STAT_EN.
*/
void startTask(void *param)
{
#if OS_TASK_STAT_EN
   OSStatInit(); // inicializace statisticke ulohy
#endif

  OSTaskCreate(keyboardTask, (void *)0, (void *)&TaskKeyStk[TASK_STK_SIZE - 1], 1);     // uloha zpracovani klavesnice
  OSTaskCreate(displayTask, (void *)0, (void *)&TaskDisplayStk[TASK_STK_SIZE - 1], 2);  // uloha zpracovani displeje
  OSTaskCreate(loadTask, (void *)0, (void *)&TaskLoadStk[TASK_STK_SIZE - 1], 3);        // zatezova uloha

  for (;;) {
    OSTimeDlyHMSM(0,0,1,0); // zpozdeni 1s
  }

}

#endif

/*
 * Hlavni funkce programu
 */
int main() {

#if FITKIT_VERSION == 1
   // Inicializace HW
   my_initialize_hardware();

   // Smer pinu diod
   P1DIR |= BIT0; // D5
   P5DIR |= BIT7; // D6

   // Vypnuti diod
   P1OUT |= BIT0;
   P5OUT |= BIT7;

#elif FITKIT_VERSION == 2
   // Inicializace HW
   initialize_hardware();
#endif

   // Inicializace OS a uloh
    OSInit();

#if FITKIT_VERSION == 1
    // Uloha blikani zelene diody
    OSTaskCreate(green_task, (void *)0, (void *)&TaskGreenLEDStk[TASK_STK_SIZE - 1], 0);
    // Uloha blikani cervene diody
    OSTaskCreate(red_task, (void *)0, (void *)&TaskRedLEDStk[TASK_STK_SIZE - 1], 1);
#elif FITKIT_VERSION == 2
    // Startovaci uloha
    OSTaskCreate(startTask, (void *)0, (void*)&TaskStartStk[TASK_STK_SIZE - 1], 0);
#endif

    // Spusteni watchdogu
    WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL; // Timer mode, predeleni 32768
    IE1 |= WDTIE;
 
    // Povoleni general interrupt
    _EINT();
 
    // Start multitaskingu
    OSStart();

   return 0;
}
