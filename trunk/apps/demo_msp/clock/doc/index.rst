.. article:: apps_demo_msp_clock
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090415

    Ukázka implementace pøesnıch hodiny reálného èasu (RTC, real-time clock) pomocí mikrokontroleru. 

=======================
Hodiny pomocí èasovaèe
=======================

.. contents:: Obsah

Popis aplikace
===============

.. image:: clock_app.png
   :align: right

Aplikace demonstruje implementaci hodin reálného èasu (RTC, real-time clock) na platformì FITkit. Pro mìøení èasu je vyuit èasovaè pracující v reimu vıstupní komparace. Èas na LCD je aktualizován v obsluze pøerušení èasovaèe. Aktualizace je provádìna s periodou 1 s a je signalizována zmìnou stavu LED D5. Vıhodou pouití èasovaèe je, e CPU mikrokontroléru nemusí kvùli odmìøování èasu aktivnì èekat ve smyèce; mìøení èasu pøenechá èasovaèi a vìnuje se jinım uiteènım èinnostem. Aplikaci je mono snadno doplnit o funkce kalendáøe, organizéru, budíku èi o funkce pro øízení událostí závislıch na èase (zavlaování ve skleníku, øízení reimu èinnosti nemovitosti atp.).

Principy související s øízením svitu D5 je moné nalézt napø. v aplikaci Blikání LED pomocí èasovaèe. Tento text bude vìnován pouze implementaci RTC a aktualizaci èasu na LCD.

Po spuštìní aplikace se na LCD objeví úvodní text, kterı tam zùstane a do okamiku první aktualizace èasu vyvolané prvním pøerušením od èasovaèe. Od tohoto okamiku bude s periodou 1 s jednak na LCD aktualizován èas ajednak bude docházet ke zmìnì stavu D5.

Úvodní text se na LCD objeví hned po naprogramování FPGA. Zajistí to funkce

:: 

 void fpga_initialized()
 {
   ...
   LCD_init();                          // inicializuj LCD
   LCD_append_string("FITkit: clock");  // posli text na LCD
   LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0); // zapni LCD, vypni kurzor
 }

Práce s èasovaèem
==================

**Konfigurace èasovaèe A0**

Jeliko chceme vyvolávat pøerušení s ``periodou 1 s``, bude nejvıhodnìjší zvolit za zdroj hodin èasovaèe hodinovı signál ``ACLK``, kterı má kmitoèet ``32768 Hz``:

::

  TACTL = TASSEL_1 + MC_2;  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

Aby bylo moné vyuít pøerušení od èasovaèe, je nutno nejprve povolit pøerušení pro èasovaè:

::

  CCTL0 = CCIE;   // povol preruseni pro casovac (rezim vystupni komparace)

a nastavit, po kolika ticích (hodin ACLK s kmitoètem 32768 Hz) má bıt vyvoláno pøerušení. Jeliko chceme vyvolat pøerušení po uplynutí 1 s, pak necháme èasovaè ``èítat do hodnoty 32768``. Poté, co èasovaè dosáhne této hodnoty, uplyne právì 1 s:

::

  CCR0 = 0x8000;                            // nastav po kolika ticich (32768 dekadicky = 0x8000 hexa, 
                                            // tj. za 1 s) ma dojit k preruseni

Dále si ukáeme, jak je moné napsat obsluhu pøerušení pro èasovaè a co bude v jejím tìle.

**Obsluha pøerušení èasovaèe A0**

Napsat obsluhu pøerušení pro èasovaè A0 znamená napsat tìlo funkce ``interrupt (TIMERA0_VECTOR) Timer_A (void)``

V pøípadì naší aplikace by obsluha pøerušení (vyvolávaná kadou 1 s za úèelem aktualizace èasu) èasovaèe A0 mohla vypadat napø. takto:

::

 interrupt (TIMERA0_VECTOR) Timer_A (void)
 {
   P1OUT ^= 0x01;      // invertuj bit P1.0 (kazdou 1/2 s)
   CCR0 += 0x8000;     // nastav po kolika ticich (32768 = 0x8000, tj. za 1 s) ma dojit k dalsimu preruseni

   clk_s++; 

   if(clk_s==60) { clk_m++; clk_s=0; }     // kazdou 60. sekundu inkrementuj minuty
   if(clk_m==60) { clk_h++; clk_m=0; }     // kazdou 60. minutu inkrementuj hodiny
   if(clk_h==24)                         // kazdych 24 hodin nuluj h,m,s
   {
     clk_h = clk_m = clk_s = 0;
   }

   flag_timer += 1;
 }

V tìle obsluhy pøerušení se nastaví pøíznak ``flag_timer`` indikující, e nastalo pøerušení. Tento pøíznak je dále testován v hlavní smyèèe programu a zpùsobí aktualizaci LCD displeje. Aktualizace by sice mohla bıt v tìle pøerušení, ale tím bychom kadou sekundu blokovali SPI kanál mezi MCU a FPGA a nebylo by moné napø. naprogramovat FPGA apod.

Aktualizace èasu
=================

Pro zobrazování èasu na LCD slouí funkce ``display_clock()``. Jejím úkolem je zobrazit na LCD èas ve formátu ``FITkit: HH:MM:SS``:

::

 void display_clock() {
   LCD_send_cmd(LCD_SET_DDRAM_ADDR | LCD_SECOND_HALF_OFS, 0); // kurzor na druhou polovinu
   delay_ms(2);

   LCD_send_cmd((unsigned char)(clk_h / 10) + 48, 1); delay_ms(2);     // zobraz hh
   LCD_send_cmd((unsigned char)(clk_h % 10) + 48, 1); delay_ms(2); 
   LCD_send_cmd(0x3A, 1); delay_ms(2);                                 // :
   LCD_send_cmd((unsigned char)(clk_m / 10) + 48, 1); delay_ms(2);     // zobraz mm
   LCD_send_cmd((unsigned char)(clk_m % 10) + 48, 1); delay_ms(2);     
   LCD_send_cmd(0x3A, 1); delay_ms(2);                                 // :
   LCD_send_cmd((unsigned char)(clk_s / 10) + 48, 1); delay_ms(2);     // zobraz ss
   LCD_send_cmd((unsigned char)(clk_s % 10) + 48, 1); delay_ms(2);     
 }

Funkce je volána v hlavní programové smyèce:

::

 int main(void)
 {
   flag_timer = 0;
   ...
   clk_h = 0;                                // inicializuj hodiny
   clk_m = 0;                                // inicializuj minuty
   clk_s = 0;                                // inicializuj sekundy

   while (1)
   {
     ...
     if (flag_timer > 0) {                 // doslo k aktualizaci casu casovacem?
       display_clock();                    // zobraz aktualizovane hh:mm:ss
       flag_timer--;
     }
   }                                  
 }

Kompletní zdrojové kódy je moné nalézt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovoznìní aplikace
========================
1. pøelote aplikaci
2. naprogramujte MCU a FPGA a spuste terminálovı program.
3. Pomocí pøíkazu ``SET`` zadaného v oknì terminálu je mono zmìnit èas bìící v aplikaci, napø.

   ::  

     SET 13:40:56

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.
