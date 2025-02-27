.. article:: apps_demo_msp_led
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090415

    Aplikace demonstruje ��zeni zelen� LED (D5) pomoc� obsluhy p�eru�en� napsan� pro �asova� timer A0 pou�it� v re�imu v�stupn� komparace. 

============================
Blik�n� LED pomoc� �asova�e
============================

Popis aplikace
=================

Tato jednoduch� aplikace demonstruje vyu�it� �asova�e z mikrokontrol�ru k odm��ov�n� frekvence blik�n� LED. V�hodou pou�it� �asova�e je, �e CPU mikrokontrol�ru nemus� kv�li odm��ov�n� �asu aktivn� �ekat ve smy�ce; m��en� �asu p�enech� �asova�i a v�nuje se jin�m u�ite�n�m �innostem.

P�ipojen� LED diody D5 k MCU
==============================

Jeliko� anoda D5 je (p�es rezistor R53) spojena s potenci�lem 3.3 V, je ze sch�matu z�ejm�, �e D5 bude zhasnuta bude-li na v�vodu P1.0 mikrokontrol�ru (spojen�m s katodou D5) stejn� potenci�l (to je p��pad log. 1 na v�vodu P1.0). Naopak, D5 bude sv�tit bude-li potenci�l na P1.0 dostate�n� men�� ne� 3.3 V (p��pad log.0 na v�vodu P1.0).

.. figure:: msp_led_d5.png

   Svit D5 je mo�no ovl�dat sign�lem z bitu 0 portu 1 (P1.0) mikrokontrol�ru MSP430 (detail sch�matu a um�st�n� LED D5 na FITkitu).  

Princip ��zen� D5 z MCU
========================

Aby bylo mo�n� D5 ovl�dat v�vodem P1.0 (tj. 0. bit portu P1), je t�eba nejprve nastavit tento v�vod na v�stupn�, co� je mo�no prov�st nap�. p��kazem

::

  P1DIR |= 0x01;   // nastav P1.0 na vystup (budeme ovladat zelenou LED D5)

Zhasnut� D5 (tj. nastaven� 0. bitu portu P1 na log.1) je pak mo�no prov�st nap�. p��kazem

::

  P1OUT |= 0x01;   // P1.0 na log.1 (D5 off)

a obdobn� rozsv�cen� D5 (tj. nastaven� 0. bitu portu P1 na log.0) p��kazem

::

  P1OUT &= 0xFE;   // P1.0 na log.0 (D5 on)

Jinou mo�nost� je pou��t m�sto uveden�ch p��kaz� funkce ``set_led_d5(1)``, ``set_led_d5(0)`` nebo ``flip_led_d5()``.

V na�� aplikaci v�ak budeme cht�t, aby na�e LED blikala s periodou 1 s, a to nez�visle na hlavn� programov� smy�ce prov�d�n� mikrokontrol�rem. 
K ��zen� LED proto vyu�ijeme �asova�, kter� je schopen v periodick�ch intervalech vyvol�vat p�eru�en�. 
V obsluze tohoto p�eru�en� budeme m�nit stav LED ze zhasnut� na rozsv�cenou a naopak.

**Konfigurace �asova�e A0**

Jeliko� chceme vyvol�vat p�eru�en� s periodou 1 s, bude nejv�hodn�j�� zvolit za zdroj hodin �asova�e hodinov� sign�l ACLK, kter� m� kmito�et 32768 Hz:

::

  TACTL = TASSEL_1 + MC_2;  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

Aby bylo mo�n� vyu��t p�eru�en� od �asova�e, je nutno nejprve povolit p�eru�en� pro �asova�:

::

  CCTL0 = CCIE;   // povol preruseni pro casovac (rezim vystupni komparace)

a nastavit, po kolika tic�ch (hodin ACLK s kmito�tem 32768 Hz) m� b�t vyvol�no p�eru�en�. Jeliko� chceme, aby D5 blikala s periodou 1 s, budeme pot�ebovat, aby byla 0,5 s (tj. po 16384 tik� hodin ACLK) zhasnut� a 0,5 s rozsv�cen�:

.. figure:: msp_led_timing.png

   �asov�n�   

::

  CCR0 = 0x4000;  // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k preruseni

D�le si uk�eme, jak je mo�n� napsat obsluhu p�eru�en� pro �asova� a co bude v jej�m t�le.

**Obsluha p�eru�en� �asova�e A0**

Napsat obsluhu p�eru�en� pro �asova� A0 znamen� napsat t�lo funkce ``interrupt (TIMERA0_VECTOR) Timer_A (void)``

V p��pad� na�� aplikace by obsluha p�eru�en� (vyvol�van� ka�dou 1/2 s za ��elem zm�ny stavu D5) �asova�e A0 mohla vypadat nap�. takto:

::

  interrupt (TIMERA0_VECTOR) Timer_A (void)
  {
    P1OUT ^= 0x01;    // invertuj bit P1.0 (kazdou 1/2 s)
    CCR0 += 0x4000;   // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k dalsimu preruseni
  }

Kompletn� zdrojov� k�dy je mo�n� nal�zt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovozn�n� aplikace
========================
1. p�elo�te aplikaci
2. naprogramujte MCU a FPGA a spus�te termin�lov� program.

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.
