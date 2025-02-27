.. article:: apps_vga_video_player
   :author: Stanislav Sigmund <xsigmu02 AT fit.vutbr.cz>
   :updated: 12.6.2009

   Aplikace pou��v� IDE �adi� s DMA p�enosem, umo��uje proch�zet adres��ovou strukturu, a p�ehr�vat video-soubory ve vlastn�m form�tu

===============================
Video p�ehr�va�
===============================

Popis aplikace
=================
Aplikace pou��v� IDE �adi� s DMA p�enosem, umo��uje proch�zet adres��ovou strukturu, a p�ehr�vat video-soubory ve vlastn�m form�tu.

.. figure:: vplayer2.png
   Blokov� zapojen� p�ehr�va�e

��d�c� obvod grafick�ho rozhran� pou��v� n�kolik ��d�c�ch port� rozhran� SPI. Jako pam�ov� m�dium je pou�ita pam� SDRAM, p�i pou�it�m rozli�en� 640x480 poskytuje 16 pam�ov�ch oblast�(bank). Data jsou ulo�ena ��dce, tedy sou��st� adresy je p��mo ��slo ��dku a sloupce.

.. table:: Popis jednotliv�ch ��d�c�ch port� SPI grafick�ho rozhran�

   +-------+-------+------------------------------------+-----------------+--------------+
   |adresa | re�im | pou�it�                            |d�lka adresy (B) | d�lka dat(B) |
   +=======+=======+====================================+=================+==============+
   |40H    | Z     |bit 7=zapnut� zobrazov�n�,          | 1               | 1            |
   |       |       |bity 3-0 ur�uj� zobrazovanou banku  |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |41H    | Z     |bity 6-0 ur�uj� horizont�ln�        | 1               | 1            |
   |       |       |rozli�en�/8 (80=640)                |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |42H    | Z     |bity 5-0 ur�uj� vertik�ln�          | 1               | 1            |
   |       |       |rozli�en�/8 (60=480)                |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |43H    | Z     |bity 1 a 0=zm�na m���tka (0=640x480,| 1               | 1            |
   |       |       |1=320x240,2=160x120,3=80x60)        |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |80H+81H| Z     |c�lov� adresa dat pro p�enos p�es   | 1               | 3            |
   |       |       |v�stupn� kan�l �adi�e: bity 9-0=    |                 |              |
   |       |       |��slo sloupce, bity 18-10= ��slo    |                 |              |
   |       |       |��dku, bity 22-19= ��slo banky      |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+

Pro p��stup k registru 80H je vhodn� pou��t fuknci FPGA_SPI_RW_A8_D3 z jednotky ide.h, nebo� funkce FPGA_SPI_RW_A8_DN p�evrac� po�ad� znak�.

Podporovan� p��kazy:
---------------------
init
  inicializuje disk a na�te 1. partition na disku
dir/dirp[ n�zev]
  zobraz� seznam v�ech polo�ek, p��padn� jen polo�ky zadan�ho jm�na (lze pou��t hv�zdi�kovou konvenci). verze dirp zobrazuje pouze jm�na
cd
  p�ejde na adres��, pracuje pouze o jednu �rove�, \ = ko�enov� adres��
play n�zev
  p�ehraje video
show n�zev
  zobraz� obr�zek

Popis video-form�tu
======================

Soubor typu RIF
---------------
Jedn� se o velice jednoduch� form�t slou��c� k ulo�en� statick�ho obr�zku. 
Obsah souboru za��n� 4-znakovou identifikac� "RIF ", d�le n�sleduje ���ka a v��ka obr�zku, oboj� o velikosti slova. 
Pot� jsou ji� ulo�ena obrazov� data, ve form�tu 3:3:2 (po�et bit� na R:G:B kan�l), v po�ad� zleva doprava a shora dol�.
Maxim�ln� rozm�ry obr�zku jsou 640x480. Obr�zek ve form�tu RIF lze je vytv��et ze soubor� typu BMP (pouze 256 barev) pomoc� utility bmp2rif dostupn� v adres��i ``tools/avi2raf``.

P��klad pou�it�: ``bmp2rif vstup.bmp v�stup.rif``
 
Soubor typu RAF
---------------
Tento soubor se pou��v� k ulo�en� videosekvenc� a za��n� hlavi�kou, kterou popisuje n�sleduj�c� tabulka.

.. table:: Popis hlavi�ky souboru RAF

   +--------+--------------+----------------------------------+
   | Offset | Velikost [B] | Popis                            |
   +========+==============+==================================+
   | 00H    | 4            |znaky "RAFF"                      |
   +--------+--------------+----------------------------------+
   | 04H    | 4            |verze (00010000H)                 |
   +--------+--------------+----------------------------------+
   | 08H    | 2            |velikost hlavi�ky (22)            |
   +--------+--------------+----------------------------------+
   | 0AH    | 2            |���ka (max. 640)                  |
   +--------+--------------+----------------------------------+
   | 0CH    | 2            |v��ka (max. 480)                  |
   +--------+--------------+----------------------------------+
   | 0EH    | 2            |rychlost (0=neudan�)              |
   +--------+--------------+----------------------------------+
   | 10H    | 4            |po�et sn�mk�                      |
   +--------+--------------+----------------------------------+
   | 14H    | 2            |typ komprese (0=bez komprese)     |
   +--------+--------------+----------------------------------+
   | 16H    | 2            |p��tomnost indexu (zat�m 0)       |
   +--------+--------------+----------------------------------+
   | 18H    | 2            |velikost zvukov�ch blok�          |
   +--------+--------------+----------------------------------+
   | 20H    | 2            |typ komprese zvuku                |
   +--------+--------------+----------------------------------+

Za hlavi�kou n�sleduj� jednotliv� sn�mky. Data jsou ulo�ena �pln� ve stejn�m form�tu jako v p��pad� RIF.
Max. velikost jednoho video sn�mku je 640x480, pro plynul� p�ehr�v�n� v�ak doporu�ujeme pou��t 160x120.
Pro konverzi video soubor� AVI lze pou��t utility ``avi2raf`` a ``bmp2raf`` v adres��i tools.

P��klad pou�it�:
* ``avi2raf zdroj.avi cil.raf`` - jsou vyu�ity standartn� API funkce, tak�e video m��e b�t i komprimovan�
* ``bmp2raf cil.raf zdroj1.bmp zdroj2.bmp ...`` - v�echny soubory typu bmp mus� m�t stejnou velikost a hloubku 256 barev

Zprovozn�n� aplikace
=======================

1. nahrajte na disk video soubor/y
2. p�ipojte pevn� disk k FITkitu (viz dokument `DOCext_ide`_) nakonfigurovan� jako MASTER. Kabel mus� b�t upraven� pro architekturu small
3. p�elo�te aplikaci
4. naprogramujte MCU a FPGA a spus�te termin�lov� program
5. aktivujte propojku J6 pro povolen� periferi� PC
6. po spu�t�n� je nutn� za��t p��kazem ``init``
7. p�ehr�v�n� spus�te p��kazem ``play nazev_souboru``
