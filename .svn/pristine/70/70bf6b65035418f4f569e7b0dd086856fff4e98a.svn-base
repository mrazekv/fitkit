.. article:: apps_demo_msp_signal_generator
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090417

    Generátor signálù s nastavitelnım kmitoètem a velikostí amplitudy napìtí

==================
Generátor signálù
==================
.. contents:: Obsah

Popis aplikace
==================

Pomocí DA pøevodníku dostupného na mikrokontroléru aplikace demonstruje jednoduchı zpùsob generování základních prùbìhù signálù (konstantní, harmonickı, pilovitı, obdélníkovı) s moností nastavení kmitoètu a amplitudy zvoleného prùbìhu. Napìtí je mìøitelné mezi vyvodem 31 (konektoru JP9) a zemí FITkitu.

.. figure:: overview.png

   Ukázka generovanıch prùbìhù (snímek obrazovky osciloskopu)

Pøíkazová øádka aplikace
=========================

Poadovanı prùbìh je mono generovat po volbì jeho parametrù pomocí pøíkazù v terminálovém oknì. Pøíkazy jsou následující:

::

  SIG harm ..... harmonicky signal
  SIG saw ...... pilovity signal
  SIG square ... obdelnikovy signal
  SIG const ... konstantni signal
  SCALE x ... zmena amplitudy napeti na x = 0 ... 100 [%]
  FREQ f ....... zmena frekvence [Hz]

DA pøevodník je nastaven do 8-bitového reimu a umoòuje generovat prùbìhy o napìovém rozmezí 0 V a 1,5 V a frekvencích v rozsahu 1 Hz a 500 Hz v závislosti na tvaru zvoleného prùbìhu.

  .. note:: Jinou konfiguraci DA pøevodníku (12-bitovı reim, referenèní napìtí 2,5 V) mùete nalézt v aplikaci `DOCapps_demo_msp_voltage_generator`_.

Napìtí je generováno na ``0. kanálu DA pøevodníku`` mikrokontroléru. Tento kanál je vyveden na ``pin è. 31 knektoru JP9`` - tzn. generované napìtí je mono mìøit mezi tímto pinem a zemí.

**Ilustrace k pouití pøíkazù**

(neuspoøádanou) posloupností pøíkazù

::

 SIG const
 SCALE 100
 
zajistíme generování konstantního signálu o hodnotì napìtí 1,5 V = 1500 mV (rovné 100 % referenèního napìtí):

.. image:: scope_1.png
   :align: center

(neuspoøádanou) posloupností pøíkazù

::

 SIG const
 SCALE 50
 
zajistíme generování konstantního signálu o hodnotì napìtí 0,775 V = 775 mV (rovné 50 % referenèního napìtí):

.. image:: scope_2.png
   :align: center

(neuspoøádanou) posloupností pøíkazù

::

 SIG HARM
 SCALE 50
 FREQ 100
 
zajistíme generování harmonického signálu o kmitoètu 100 Hz s rozkmitem napìtí 0,775 V = 775 mV (rovno 50 % referenèního napìtí):

.. image:: scope_3.png
   :align: center

(neuspoøádanou) posloupností pøíkazù

::

 SIG HARM
 SCALE 50
 FREQ 50
 
zajistíme generování harmonického signálu o kmitoètu 50 Hz s rozkmitem napìtí 0,775 V = 775 mV (rovno 50 % referenèního napìtí):

.. image:: scope_4.png
   :align: center

(neuspoøádanou) posloupností pøíkazù

::

 SIG HARM
 SCALE 100
 FREQ 510
 
zajistíme generování harmonického signálu o kmitoètu 510 Hz s rozkmitem napìtí 1,5 V = 1500 mV (rovno 100 % referenèního napìtí):

.. image:: scope_5.png
   :align: center

(neuspoøádanou) posloupností pøíkazù

::

 SIG SAW
 SCALE 100
 FREQ 510
 
zajistíme generování pilovitého signálu o kmitoètu 510 Hz s rozkmitem napìtí 1,5 V = 1500 mV (rovno 100 % referenèního napìtí):

.. image:: scope_6.png
   :align: center

(neuspoøádanou) posloupností pøíkazù

::

 SIG SQUARE
 SCALE 100
 FREQ 100
 
zajistíme generování obdélníkového signálu o kmitoètu 100 Hz, støídou 1:1 a rozkmitem napìtí 1,5 V = 1500 mV (rovno 100 % referenèního napìtí):

.. image:: scope_7.png
   :align: center

Tvary signálù
==============

Typy tvarù podporovanıch signálù jsou umístìny ve vıètovém typu

::

 enum SIG_ID {SIG_CONST, SIG_HARM, SIG_SAW, SIG_SQUARE} sig_type;

Tvary podporovanıch signálù jsou umístìny v polích 

::

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

ve formì posloupnosti 8-bitovıch vzorkù, jejich hodnota je pøímo (lineárnì) úmìrná hodnotì generovaného napìtí.
Délky posloupností je tøeba (pro korektní pohyb v polích) nastavit na správné hodnoty, tj.::

#define SIN_SAMPLES 32    // pocet vzorku harmonickeho signalu (sinus)
#define SAW_SAMPLES 32    // pocet vzorku piloviteho signalu
#define SQUARE_SAMPLES 2  // pocet vzorku obdelnikoveho signalu
#define CONST_SAMPLES 1   // pocet vzorku konstantniho signalu

**Modifikace a doplnìní tvarù**

Modifikací stávajících, popø. doplnìním dalších polí lze generovat jiné signály - napø. obdélníkovı signál se støídou 1:4:

.. image:: scope_8.png
   :align: center


Princip generování
===================

Pro generování prùbìhù na základì hodnot v polích je vyuit èasovaè èítající na kmitoètu 32768 Hz. Ten je nastaven tak, aby po kadıch ``ticks`` ticích, kde

::

  ticks = TICKS_PER_SECOND / frequency / samples;

bylo vyvoláno pøerušení, ve kterém bude (pro danı typ prùbìhu) vybrán další byte z pøíslušného pole. 

  .. note:: Pro detailnìjší seznámení se s èasovaèem viz aplikace `DOCapps_demo_msp_led`_ nebo `DOCapps_demo_msp_clock`_.

V obsluze pøerušení èasovaèe

::

 interrupt (TIMERA0_VECTOR) Timer_A (void)
 {  
   ...
   DAC12_0DAT = smpl;  // nahrani dalsiho vzorku pro prevod
   ...
 }

je tento byte uloen do registru DAC12_0DAT, èím je zahájena konverze bytu na napìtí na vıstupu AD pøevodníku. 

Práce s DA pøevodníkem
=======================

**Nastavení DA pøevodníku**

DA pøevodník pøevádí n-bitovı (tj. èíslicovı, v našem pøípadì n=8) vzorek na napìtí ve zvoleném rozsahu (tzv. referenèního napìtí, v našem pøípadì 0 V - 1,5 V). Tento pøevod je lineární, pøièem hodnotì 0x00 (0 na 8 bitech) odpovídá 0 V a hodnotì 0xff (samé 1 na 8 bitech) 1,5 V.
Referenèní napìtí mùeme nechat násobit 1x (náš pøípad) nebo 3x.

Ve funkci ``main`` je DA pøevodník nastaven takto:

::

  int main(void)
  {
    ...
    ADC12CTL0 |= 0x0020;    // nastaveni referencniho napeti na 1,5 V
    DAC12_0CTL |= 0x1060;   // nastaveni kontrolniho registru DAC (8-bitovy rezim, medium speed)
    DAC12_0CTL |= 0x100;    // referencni napeti nasobit 1x
    ..
  }

Kompletní zdrojové kódy je moné nalézt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovoznìní aplikace
========================
1. pøelote aplikaci

2. naprogramujte MCU a FPGA a spuste terminálovı program.

3. generovanı signál je mìøitelnı mezi vıvodem 31 konektoru JP9 a zemí

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.

