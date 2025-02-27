.. article:: apps_demo_msp_display
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090415

    Aplikace umoòující seznámit se s principy ovládání LCD displeje pomocí instrukcí jeho øadièe.

===================
Test instrukcí LCD
===================
.. contents:: Obsah

Popis aplikace
==================

Jednoduchá aplikace umoòující bez nutnosti jakéhokoliv programování otestovat èinnost instrukcí podporovanıch LCD. Vhodné pro základní seznámení se s LCD.  

.. figure:: lcd_app.png

   Vıchozí stav LCD

Instrukce LCD
==============

K ovládání LCD je mono vyuít následujících instrukcí kódovanıch na 8 bitech dle následující tabulky:

.. figure:: lcd_instr.png
   
   Formát LCD instrukcí

Pomocí tìchto instrukcí je mono napø. smazat obsah LCD, vypnout/zapnout LCD èi kurzor, mìnit chování kurzoru, posouvat kurzor èi obsah LCD doleva/doprava. Aby mohla bıt instrukce LCD provedena, je nutno v oknì terminálu odeslat kód instrukce pomocí pøíkazu ``CMD`` (viz níe).

Bude-li tøeba na LCD zapsat data (vdy se tak dìje na aktuální pozici kurzoru, a to bez ohledu na to je-li èi není-li viditelnı), existují dvì monosti, a to pouití pøíkazu ``DTA`` nebo pøíkazu ``DTB`` (viz níe). Pøíkaz ``DTA`` umoòuje zaslat na LCD pøímo znak vepsanı do terminálového okna, zatímco druhı pøíkaz umoòuje zaslat na LCD znak urèenı v terminálovém oknì jeho adresou v tabulce znakù LCD.

Do DDRAM pamìti LCD je mono uloit a 80 znakù, z nich však LCD dokáe (v daném reimu) zobrazit pouhıch 16 znakù - které to jsou, závisí na aktuální pozici kurzoru. ``Dùleité: znaky zobrazované na LCD jsou ukládány do DDRAM pamìti (adresa A6:A0) LCD takto:``

od ``0h`` (nejlevìjší znak levé poloviny) a po ``40h-1 = 63`` (nejpravìjší znak levé poloviny) - pøed zápisem znaku do levé poloviny LCD je tedy tøeba nejprve nastavit adresu v rámci levé poloviny (``bit A6 je nulovı``, bity A5:A0 udávají ofset v rámci levé poloviny),

od ``40h`` (nejlevìjší znak pravé poloviny) a po ``80h-1 = 127`` (nejpravìjší znak pravé poloviny) - pøed zápisem znaku do pravé poloviny LCD je tedy tøeba nejprve nastavit adresu v rámci pravé poloviny (``bit A6 je jednièkovı``, bity A5:A0 udávají ofset v rámci pravé poloviny).

.. figure:: lcd_charset.png
   
   Implicitní mapa znakù zobrazitelnıch na LCD

Zprovoznìní aplikace
========================
1. pøelote aplikaci

2. naprogramujte MCU a FPGA a spuste terminálovı program.

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.

Pøíkazová øádka aplikace
=========================

Po navázání komunikace mezi PC a FITkitem je LCD displej mono øídít pøímo z terminálového okna, a to pomocí pøíkazù:

::

  CMD X  ... zaslani instrukce X (8-bitova binarni data) do LCD
         ... priklad pro smazani LCD displeje: CMD 00000001
         ... priklad pro posun kurzoru do vychozi pozice: CMD 00000010
  DTA X  ... zaslani dat X (znak z tabulky znaku LCD) do LCD
         ... priklad pro zaslani znaku '3' na LCD: DTA 3
         ... priklad pro zaslani znaku '*' na LCD: DTA *
  DTB X  ... zaslani dat X (8-bitovy kod znaku z tabulky znaku LCD) do LCD
         ... priklad pro zaslani znaku '3' na LCD: DTB 00110011
         ... priklad pro zaslani znaku '*' na LCD: DTB 00101010
  
  Binarni data odpovidaji (zleva doprava) hodnotam bitu DB7:DB0 rozhrani LCD

**Ilustrace k pouití pøíkazové øádky**

Následující obrázek ilustruje obsah LCD po provedení posloupnosti pøíkazù

::

  DTA H
  DTA I
  DTA !

odeslání øetìzce HI! na LCD; stav LCD po pøijetí pøíkazu øadièem LCD je:

.. image:: lcd_ex1.png
   :align: center

::

  CMD 00000010

tj. zaslání instrukce 00000010 (posun kurzoru do vıchozí pozice) do LCD; stav LCD po pøijetí pøíkazu øadièem LCD je:

.. image:: lcd_ex2.png
   :align: center

::

  CMD 00000001

tj. smazání LCD instrukcí 00000001; stav LCD po pøijetí pøíkazu øadièem LCD je:

.. image:: lcd_ex3.png
   :align: center

::

  DTB 11100000

tj. zapsání øeckého písmene malá alfa na LCD; stav LCD po pøijetí pøíkazu øadièem LCD je:

.. image:: lcd_ex4.png
   :align: center

::

  DTA 1
  DTA 3
  DTB 11011111
  DTA C

tj. zapsání údaje o teplotì na LCD; stav LCD po pøijetí pøíkazu øadièem LCD je:

.. image:: lcd_ex5.png
   :align: center

::

  DTA 6
  DTB 11111101
  DTA 3
  DTA =
  DTA 2

tj. zápisu jednoduchého aritmetického vırazu na LCD; stav LCD po pøijetí pøíkazu øadièem LCD je:

.. image:: lcd_ex6.png
   :align: center

