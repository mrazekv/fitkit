.. article:: apps_demo_msp_display
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090415

    Aplikace umo��uj�c� sezn�mit se s principy ovl�d�n� LCD displeje pomoc� instrukc� jeho �adi�e.

===================
Test instrukc� LCD
===================
.. contents:: Obsah

Popis aplikace
==================

Jednoduch� aplikace umo��uj�c� bez nutnosti jak�hokoliv programov�n� otestovat �innost instrukc� podporovan�ch LCD. Vhodn� pro z�kladn� sezn�men� se s LCD.  

.. figure:: lcd_app.png

   V�choz� stav LCD

Instrukce LCD
==============

K ovl�d�n� LCD je mo�no vyu��t n�sleduj�c�ch instrukc� k�dovan�ch na 8 bitech dle n�sleduj�c� tabulky:

.. figure:: lcd_instr.png
   
   Form�t LCD instrukc�

Pomoc� t�chto instrukc� je mo�no nap�. smazat obsah LCD, vypnout/zapnout LCD �i kurzor, m�nit chov�n� kurzoru, posouvat kurzor �i obsah LCD doleva/doprava. Aby mohla b�t instrukce LCD provedena, je nutno v okn� termin�lu odeslat k�d instrukce pomoc� p��kazu ``CMD`` (viz n�e).

Bude-li t�eba na LCD zapsat data (v�dy se tak d�je na aktu�ln� pozici kurzoru, a to bez ohledu na to je-li �i nen�-li viditeln�), existuj� dv� mo�nosti, a to pou�it� p��kazu ``DTA`` nebo p��kazu ``DTB`` (viz n�e). P��kaz ``DTA`` umo��uje zaslat na LCD p��mo znak vepsan� do termin�lov�ho okna, zat�mco druh� p��kaz umo��uje zaslat na LCD znak ur�en� v termin�lov�m okn� jeho adresou v tabulce znak� LCD.

Do DDRAM pam�ti LCD je mo�no ulo�it a� 80 znak�, z nich� v�ak LCD dok�e (v dan�m re�imu) zobrazit pouh�ch 16 znak� - kter� to jsou, z�vis� na aktu�ln� pozici kurzoru. ``D�le�it�: znaky zobrazovan� na LCD jsou ukl�d�ny do DDRAM pam�ti (adresa A6:A0) LCD takto:``

od ``0h`` (nejlev�j�� znak lev� poloviny) a� po ``40h-1 = 63`` (nejprav�j�� znak lev� poloviny) - p�ed z�pisem znaku do lev� poloviny LCD je tedy t�eba nejprve nastavit adresu v r�mci lev� poloviny (``bit A6 je nulov�``, bity A5:A0 ud�vaj� ofset v r�mci lev� poloviny),

od ``40h`` (nejlev�j�� znak prav� poloviny) a� po ``80h-1 = 127`` (nejprav�j�� znak prav� poloviny) - p�ed z�pisem znaku do prav� poloviny LCD je tedy t�eba nejprve nastavit adresu v r�mci prav� poloviny (``bit A6 je jedni�kov�``, bity A5:A0 ud�vaj� ofset v r�mci prav� poloviny).

.. figure:: lcd_charset.png
   
   Implicitn� mapa znak� zobraziteln�ch na LCD

Zprovozn�n� aplikace
========================
1. p�elo�te aplikaci

2. naprogramujte MCU a FPGA a spus�te termin�lov� program.

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.

P��kazov� ��dka aplikace
=========================

Po nav�z�n� komunikace mezi PC a FITkitem je LCD displej mo�no ��d�t p��mo z termin�lov�ho okna, a to pomoc� p��kaz�:

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

**Ilustrace k pou�it� p��kazov� ��dky**

N�sleduj�c� obr�zek ilustruje obsah LCD po proveden� posloupnosti p��kaz�

::

  DTA H
  DTA I
  DTA !

odesl�n� �et�zce HI! na LCD; stav LCD po p�ijet� p��kazu �adi�em LCD je:

.. image:: lcd_ex1.png
   :align: center

::

  CMD 00000010

tj. zasl�n� instrukce 00000010 (posun kurzoru do v�choz� pozice) do LCD; stav LCD po p�ijet� p��kazu �adi�em LCD je:

.. image:: lcd_ex2.png
   :align: center

::

  CMD 00000001

tj. smaz�n� LCD instrukc� 00000001; stav LCD po p�ijet� p��kazu �adi�em LCD je:

.. image:: lcd_ex3.png
   :align: center

::

  DTB 11100000

tj. zaps�n� �eck�ho p�smene mal� alfa na LCD; stav LCD po p�ijet� p��kazu �adi�em LCD je:

.. image:: lcd_ex4.png
   :align: center

::

  DTA 1
  DTA 3
  DTB 11011111
  DTA C

tj. zaps�n� �daje o teplot� na LCD; stav LCD po p�ijet� p��kazu �adi�em LCD je:

.. image:: lcd_ex5.png
   :align: center

::

  DTA 6
  DTB 11111101
  DTA 3
  DTA =
  DTA 2

tj. z�pisu jednoduch�ho aritmetick�ho v�razu na LCD; stav LCD po p�ijet� p��kazu �adi�em LCD je:

.. image:: lcd_ex6.png
   :align: center

