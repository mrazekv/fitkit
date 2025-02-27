.. article:: apps_demo_msp_signal_generator
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090417

    Gener�tor sign�l� s nastaviteln�m kmito�tem a velikost� amplitudy nap�t�

==================
Gener�tor sign�l�
==================
.. contents:: Obsah

Popis aplikace
==================

Pomoc� DA p�evodn�ku dostupn�ho na mikrokontrol�ru aplikace demonstruje jednoduch� zp�sob generov�n� z�kladn�ch pr�b�h� sign�l� (konstantn�, harmonick�, pilovit�, obd�ln�kov�) s mo�nost� nastaven� kmito�tu a amplitudy zvolen�ho pr�b�hu. Nap�t� je m��iteln� mezi vyvodem 31 (konektoru JP9) a zem� FITkitu.

.. figure:: overview.png

   Uk�zka generovan�ch pr�b�h� (sn�mek obrazovky osciloskopu)

P��kazov� ��dka aplikace
=========================

Po�adovan� pr�b�h je mo�no generovat po volb� jeho parametr� pomoc� p��kaz� v termin�lov�m okn�. P��kazy jsou n�sleduj�c�:

::

  SIG harm ..... harmonicky signal
  SIG saw ...... pilovity signal
  SIG square ... obdelnikovy signal
  SIG const ... konstantni signal
  SCALE x ... zmena amplitudy napeti na x = 0 ... 100 [%]
  FREQ f ....... zmena frekvence [Hz]

DA p�evodn�k je nastaven do 8-bitov�ho re�imu a umo��uje generovat pr�b�hy o nap�ov�m rozmez� 0 V a� 1,5 V a frekvenc�ch v rozsahu 1 Hz a� 500 Hz v z�vislosti na tvaru zvolen�ho pr�b�hu.

  .. note:: Jinou konfiguraci DA p�evodn�ku (12-bitov� re�im, referen�n� nap�t� 2,5 V) m��ete nal�zt v aplikaci `DOCapps_demo_msp_voltage_generator`_.

Nap�t� je generov�no na ``0. kan�lu DA p�evodn�ku`` mikrokontrol�ru. Tento kan�l je vyveden na ``pin �. 31 knektoru JP9`` - tzn. generovan� nap�t� je mo�no m��it mezi t�mto pinem a zem�.

**Ilustrace k pou�it� p��kaz�**

(neuspo��danou) posloupnost� p��kaz�

::

 SIG const
 SCALE 100
 
zajist�me generov�n� konstantn�ho sign�lu o hodnot� nap�t� 1,5 V = 1500 mV (rovn� 100 % referen�n�ho nap�t�):

.. image:: scope_1.png
   :align: center

(neuspo��danou) posloupnost� p��kaz�

::

 SIG const
 SCALE 50
 
zajist�me generov�n� konstantn�ho sign�lu o hodnot� nap�t� 0,775 V = 775 mV (rovn� 50 % referen�n�ho nap�t�):

.. image:: scope_2.png
   :align: center

(neuspo��danou) posloupnost� p��kaz�

::

 SIG HARM
 SCALE 50
 FREQ 100
 
zajist�me generov�n� harmonick�ho sign�lu o kmito�tu 100 Hz s rozkmitem nap�t� 0,775 V = 775 mV (rovno 50 % referen�n�ho nap�t�):

.. image:: scope_3.png
   :align: center

(neuspo��danou) posloupnost� p��kaz�

::

 SIG HARM
 SCALE 50
 FREQ 50
 
zajist�me generov�n� harmonick�ho sign�lu o kmito�tu 50 Hz s rozkmitem nap�t� 0,775 V = 775 mV (rovno 50 % referen�n�ho nap�t�):

.. image:: scope_4.png
   :align: center

(neuspo��danou) posloupnost� p��kaz�

::

 SIG HARM
 SCALE 100
 FREQ 510
 
zajist�me generov�n� harmonick�ho sign�lu o kmito�tu 510 Hz s rozkmitem nap�t� 1,5 V = 1500 mV (rovno 100 % referen�n�ho nap�t�):

.. image:: scope_5.png
   :align: center

(neuspo��danou) posloupnost� p��kaz�

::

 SIG SAW
 SCALE 100
 FREQ 510
 
zajist�me generov�n� pilovit�ho sign�lu o kmito�tu 510 Hz s rozkmitem nap�t� 1,5 V = 1500 mV (rovno 100 % referen�n�ho nap�t�):

.. image:: scope_6.png
   :align: center

(neuspo��danou) posloupnost� p��kaz�

::

 SIG SQUARE
 SCALE 100
 FREQ 100
 
zajist�me generov�n� obd�ln�kov�ho sign�lu o kmito�tu 100 Hz, st��dou 1:1 a rozkmitem nap�t� 1,5 V = 1500 mV (rovno 100 % referen�n�ho nap�t�):

.. image:: scope_7.png
   :align: center

Tvary sign�l�
==============

Typy tvar� podporovan�ch sign�l� jsou um�st�ny ve v��tov�m typu

::

 enum SIG_ID {SIG_CONST, SIG_HARM, SIG_SAW, SIG_SQUARE} sig_type;

Tvary podporovan�ch sign�l� jsou um�st�ny v pol�ch 

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

ve form� posloupnosti 8-bitov�ch vzork�, jejich� hodnota je p��mo (line�rn�) �m�rn� hodnot� generovan�ho nap�t�.
D�lky posloupnost� je t�eba (pro korektn� pohyb v pol�ch) nastavit na spr�vn� hodnoty, tj.::

#define SIN_SAMPLES 32    // pocet vzorku harmonickeho signalu (sinus)
#define SAW_SAMPLES 32    // pocet vzorku piloviteho signalu
#define SQUARE_SAMPLES 2  // pocet vzorku obdelnikoveho signalu
#define CONST_SAMPLES 1   // pocet vzorku konstantniho signalu

**Modifikace a dopln�n� tvar�**

Modifikac� st�vaj�c�ch, pop�. dopln�n�m dal��ch pol� lze generovat jin� sign�ly - nap�. obd�ln�kov� sign�l se st��dou 1:4:

.. image:: scope_8.png
   :align: center


Princip generov�n�
===================

Pro generov�n� pr�b�h� na z�klad� hodnot v pol�ch je vyu�it �asova� ��taj�c� na kmito�tu 32768 Hz. Ten je nastaven tak, aby po ka�d�ch ``ticks`` tic�ch, kde

::

  ticks = TICKS_PER_SECOND / frequency / samples;

bylo vyvol�no p�eru�en�, ve kter�m bude (pro dan� typ pr�b�hu) vybr�n dal�� byte z p��slu�n�ho pole. 

  .. note:: Pro detailn�j�� sezn�men� se s �asova�em viz aplikace `DOCapps_demo_msp_led`_ nebo `DOCapps_demo_msp_clock`_.

V obsluze p�eru�en� �asova�e

::

 interrupt (TIMERA0_VECTOR) Timer_A (void)
 {  
   ...
   DAC12_0DAT = smpl;  // nahrani dalsiho vzorku pro prevod
   ...
 }

je tento byte ulo�en do registru DAC12_0DAT, ��m� je zah�jena konverze bytu na nap�t� na v�stupu AD p�evodn�ku. 

Pr�ce s DA p�evodn�kem
=======================

**Nastaven� DA p�evodn�ku**

DA p�evodn�k p�ev�d� n-bitov� (tj. ��slicov�, v na�em p��pad� n=8) vzorek na nap�t� ve zvolen�m rozsahu (tzv. referen�n�ho nap�t�, v na�em p��pad� 0 V - 1,5 V). Tento p�evod je line�rn�, p�i�em� hodnot� 0x00 (0 na 8 bitech) odpov�d� 0 V a hodnot� 0xff (sam� 1 na 8 bitech) 1,5 V.
Referen�n� nap�t� m��eme nechat n�sobit 1x (n� p��pad) nebo 3x.

Ve funkci ``main`` je DA p�evodn�k nastaven takto:

::

  int main(void)
  {
    ...
    ADC12CTL0 |= 0x0020;    // nastaveni referencniho napeti na 1,5 V
    DAC12_0CTL |= 0x1060;   // nastaveni kontrolniho registru DAC (8-bitovy rezim, medium speed)
    DAC12_0CTL |= 0x100;    // referencni napeti nasobit 1x
    ..
  }

Kompletn� zdrojov� k�dy je mo�n� nal�zt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovozn�n� aplikace
========================
1. p�elo�te aplikaci

2. naprogramujte MCU a FPGA a spus�te termin�lov� program.

3. generovan� sign�l je m��iteln� mezi v�vodem 31 konektoru JP9 a zem�

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.

