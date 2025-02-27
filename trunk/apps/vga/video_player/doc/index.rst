.. article:: apps_vga_video_player
   :author: Stanislav Sigmund <xsigmu02 AT fit.vutbr.cz>
   :updated: 12.6.2009

   Aplikace pouívá IDE øadiè s DMA pøenosem, umoòuje procházet adresáøovou strukturu, a pøehrávat video-soubory ve vlastním formátu

===============================
Video pøehrávaè
===============================

Popis aplikace
=================
Aplikace pouívá IDE øadiè s DMA pøenosem, umoòuje procházet adresáøovou strukturu, a pøehrávat video-soubory ve vlastním formátu.

.. figure:: vplayer2.png
   Blokové zapojení pøehrávaèe

Øídící obvod grafického rozhraní pouívá nìkolik øídících portù rozhraní SPI. Jako pamìové médium je pouita pamì SDRAM, pøi pouitém rozlišení 640x480 poskytuje 16 pamìovıch oblastí(bank). Data jsou uloena øídce, tedy souèástí adresy je pøímo èíslo øádku a sloupce.

.. table:: Popis jednotlivıch øídících portù SPI grafického rozhraní

   +-------+-------+------------------------------------+-----------------+--------------+
   |adresa | reim | pouití                            |délka adresy (B) | délka dat(B) |
   +=======+=======+====================================+=================+==============+
   |40H    | Z     |bit 7=zapnutí zobrazování,          | 1               | 1            |
   |       |       |bity 3-0 urèují zobrazovanou banku  |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |41H    | Z     |bity 6-0 urèují horizontální        | 1               | 1            |
   |       |       |rozlišení/8 (80=640)                |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |42H    | Z     |bity 5-0 urèují vertikální          | 1               | 1            |
   |       |       |rozlišení/8 (60=480)                |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |43H    | Z     |bity 1 a 0=zmìna mìøítka (0=640x480,| 1               | 1            |
   |       |       |1=320x240,2=160x120,3=80x60)        |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+
   |80H+81H| Z     |cílová adresa dat pro pøenos pøes   | 1               | 3            |
   |       |       |vıstupní kanál øadièe: bity 9-0=    |                 |              |
   |       |       |èíslo sloupce, bity 18-10= èíslo    |                 |              |
   |       |       |øádku, bity 22-19= èíslo banky      |                 |              |
   +-------+-------+------------------------------------+-----------------+--------------+

Pro pøístup k registru 80H je vhodné pouít fuknci FPGA_SPI_RW_A8_D3 z jednotky ide.h, nebo funkce FPGA_SPI_RW_A8_DN pøevrací poøadí znakù.

Podporované pøíkazy:
---------------------
init
  inicializuje disk a naète 1. partition na disku
dir/dirp[ název]
  zobrazí seznam všech poloek, pøípadnì jen poloky zadaného jména (lze pouít hvìzdièkovou konvenci). verze dirp zobrazuje pouze jména
cd
  pøejde na adresáø, pracuje pouze o jednu úroveò, \ = koøenovı adresáø
play název
  pøehraje video
show název
  zobrazí obrázek

Popis video-formátu
======================

Soubor typu RIF
---------------
Jedná se o velice jednoduchı formát slouící k uloení statického obrázku. 
Obsah souboru zaèíná 4-znakovou identifikací "RIF ", dále následuje šíøka a vıška obrázku, obojí o velikosti slova. 
Poté jsou ji uloena obrazová data, ve formátu 3:3:2 (poèet bitù na R:G:B kanál), v poøadí zleva doprava a shora dolù.
Maximální rozmìry obrázku jsou 640x480. Obrázek ve formátu RIF lze je vytváøet ze souborù typu BMP (pouze 256 barev) pomocí utility bmp2rif dostupné v adresáøi ``tools/avi2raf``.

Pøíklad pouití: ``bmp2rif vstup.bmp vıstup.rif``
 
Soubor typu RAF
---------------
Tento soubor se pouívá k uloení videosekvencí a zaèíná hlavièkou, kterou popisuje následující tabulka.

.. table:: Popis hlavièky souboru RAF

   +--------+--------------+----------------------------------+
   | Offset | Velikost [B] | Popis                            |
   +========+==============+==================================+
   | 00H    | 4            |znaky "RAFF"                      |
   +--------+--------------+----------------------------------+
   | 04H    | 4            |verze (00010000H)                 |
   +--------+--------------+----------------------------------+
   | 08H    | 2            |velikost hlavièky (22)            |
   +--------+--------------+----------------------------------+
   | 0AH    | 2            |šíøka (max. 640)                  |
   +--------+--------------+----------------------------------+
   | 0CH    | 2            |vıška (max. 480)                  |
   +--------+--------------+----------------------------------+
   | 0EH    | 2            |rychlost (0=neudaná)              |
   +--------+--------------+----------------------------------+
   | 10H    | 4            |poèet snímkù                      |
   +--------+--------------+----------------------------------+
   | 14H    | 2            |typ komprese (0=bez komprese)     |
   +--------+--------------+----------------------------------+
   | 16H    | 2            |pøítomnost indexu (zatím 0)       |
   +--------+--------------+----------------------------------+
   | 18H    | 2            |velikost zvukovıch blokù          |
   +--------+--------------+----------------------------------+
   | 20H    | 2            |typ komprese zvuku                |
   +--------+--------------+----------------------------------+

Za hlavièkou následují jednotlivé snímky. Data jsou uloena úplnì ve stejném formátu jako v pøípadì RIF.
Max. velikost jednoho video snímku je 640x480, pro plynulé pøehrávání však doporuèujeme pouít 160x120.
Pro konverzi video souborù AVI lze pouít utility ``avi2raf`` a ``bmp2raf`` v adresáøi tools.

Pøíklad pouití:
* ``avi2raf zdroj.avi cil.raf`` - jsou vyuity standartní API funkce, take video mùe bıt i komprimované
* ``bmp2raf cil.raf zdroj1.bmp zdroj2.bmp ...`` - všechny soubory typu bmp musí mít stejnou velikost a hloubku 256 barev

Zprovoznìní aplikace
=======================

1. nahrajte na disk video soubor/y
2. pøipojte pevnı disk k FITkitu (viz dokument `DOCext_ide`_) nakonfigurovanı jako MASTER. Kabel musí bıt upravenı pro architekturu small
3. pøelote aplikaci
4. naprogramujte MCU a FPGA a spuste terminálovı program
5. aktivujte propojku J6 pro povolení periferií PC
6. po spuštìní je nutné zaèít pøíkazem ``init``
7. pøehrávání spuste pøíkazem ``play nazev_souboru``
