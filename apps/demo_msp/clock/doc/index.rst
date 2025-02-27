.. article:: apps_demo_msp_clock
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090415

    Uk�zka implementace p�esn�ch hodiny re�ln�ho �asu (RTC, real-time clock) pomoc� mikrokontroleru. 

=======================
Hodiny pomoc� �asova�e
=======================

.. contents:: Obsah

Popis aplikace
===============

.. image:: clock_app.png
   :align: right

Aplikace demonstruje implementaci hodin re�ln�ho �asu (RTC, real-time clock) na platform� FITkit. Pro m��en� �asu je vyu�it �asova� pracuj�c� v re�imu v�stupn� komparace. �as na LCD je aktualizov�n v obsluze p�eru�en� �asova�e. Aktualizace je prov�d�na s periodou 1 s a je signalizov�na zm�nou stavu LED D5. V�hodou pou�it� �asova�e je, �e CPU mikrokontrol�ru nemus� kv�li odm��ov�n� �asu aktivn� �ekat ve smy�ce; m��en� �asu p�enech� �asova�i a v�nuje se jin�m u�ite�n�m �innostem. Aplikaci je mo�no snadno doplnit o funkce kalend��e, organiz�ru, bud�ku �i o funkce pro ��zen� ud�lost� z�visl�ch na �ase (zavla�ov�n� ve sklen�ku, ��zen� re�imu �innosti nemovitosti atp.).

Principy souvisej�c� s ��zen�m svitu D5 je mo�n� nal�zt nap�. v aplikaci Blik�n� LED pomoc� �asova�e. Tento text bude v�nov�n pouze implementaci RTC a aktualizaci �asu na LCD.

Po spu�t�n� aplikace se na LCD objev� �vodn� text, kter� tam z�stane a� do okam�iku prvn� aktualizace �asu vyvolan� prvn�m p�eru�en�m od �asova�e. Od tohoto okam�iku bude s periodou 1 s jednak na LCD aktualizov�n �as ajednak bude doch�zet ke zm�n� stavu D5.

�vodn� text se na LCD objev� hned po naprogramov�n� FPGA. Zajist� to funkce

:: 

 void fpga_initialized()
 {
   ...
   LCD_init();                          // inicializuj LCD
   LCD_append_string("FITkit: clock");  // posli text na LCD
   LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0); // zapni LCD, vypni kurzor
 }

Pr�ce s �asova�em
==================

**Konfigurace �asova�e A0**

Jeliko� chceme vyvol�vat p�eru�en� s ``periodou 1 s``, bude nejv�hodn�j�� zvolit za zdroj hodin �asova�e hodinov� sign�l ``ACLK``, kter� m� kmito�et ``32768 Hz``:

::

  TACTL = TASSEL_1 + MC_2;  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

Aby bylo mo�n� vyu��t p�eru�en� od �asova�e, je nutno nejprve povolit p�eru�en� pro �asova�:

::

  CCTL0 = CCIE;   // povol preruseni pro casovac (rezim vystupni komparace)

a nastavit, po kolika tic�ch (hodin ACLK s kmito�tem 32768 Hz) m� b�t vyvol�no p�eru�en�. Jeliko� chceme vyvolat p�eru�en� po uplynut� 1 s, pak nech�me �asova� ``��tat do hodnoty 32768``. Pot�, co �asova� dos�hne t�to hodnoty, uplyne pr�v� 1 s:

::

  CCR0 = 0x8000;                            // nastav po kolika ticich (32768 dekadicky = 0x8000 hexa, 
                                            // tj. za 1 s) ma dojit k preruseni

D�le si uk�eme, jak je mo�n� napsat obsluhu p�eru�en� pro �asova� a co bude v jej�m t�le.

**Obsluha p�eru�en� �asova�e A0**

Napsat obsluhu p�eru�en� pro �asova� A0 znamen� napsat t�lo funkce ``interrupt (TIMERA0_VECTOR) Timer_A (void)``

V p��pad� na�� aplikace by obsluha p�eru�en� (vyvol�van� ka�dou 1 s za ��elem aktualizace �asu) �asova�e A0 mohla vypadat nap�. takto:

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

V t�le obsluhy p�eru�en� se nastav� p��znak ``flag_timer`` indikuj�c�, �e nastalo p�eru�en�. Tento p��znak je d�le testov�n v hlavn� smy��e programu a zp�sob� aktualizaci LCD displeje. Aktualizace by sice mohla b�t v t�le p�eru�en�, ale t�m bychom ka�dou sekundu blokovali SPI kan�l mezi MCU a FPGA a nebylo by mo�n� nap�. naprogramovat FPGA apod.

Aktualizace �asu
=================

Pro zobrazov�n� �asu na LCD slou�� funkce ``display_clock()``. Jej�m �kolem je zobrazit na LCD �as ve form�tu ``FITkit: HH:MM:SS``:

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

Funkce je vol�na v hlavn� programov� smy�ce:

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

Kompletn� zdrojov� k�dy je mo�n� nal�zt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovozn�n� aplikace
========================
1. p�elo�te aplikaci
2. naprogramujte MCU a FPGA a spus�te termin�lov� program.
3. Pomoc� p��kazu ``SET`` zadan�ho v okn� termin�lu je mo�no zm�nit �as b��c� v aplikaci, nap�.

   ::  

     SET 13:40:56

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.
