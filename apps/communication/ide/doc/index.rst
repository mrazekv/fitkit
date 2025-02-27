.. article:: apps_communication_ide
   :author: Stanislav Sigmund <xsigmu02 AT fit.vutbr.cz>
   :updated: 12.6.2009

   Aplikace umoòující pøistupovat k pevnému disku, procházet a mìnit adresáøovou strukturu a soubory

===============================
Obsluha IDE disku
===============================

Popis aplikace
=================
Aplikace pouívá IDE øadiè s DMA pøenosem, umoòuje procházet adresáøovou strukturu, vytváøet/mazat adresáøe a prohlíet/modifikovat obsah souborù.

Podporované pøíkazy:
---------------------
init
  inicializuje disk a naète 1. partition na disku
close
  uloí údaje o ovlném místì, vhodné provést po zápisu na disk pøed ukonèením aplikace
dir/dirp[ jméno]
  zobrazí seznam všech poloek, pøípadnì jen poloky zadaného jména (lze pouít hvìzdièkovou konvenci). verze dirp zobrazuje pouze jména
mkdir jméno
  vytvoøí adresáø
cd
  pøejde na adresáø, pracuje pouze o jednu úroveò, \ = koøenovı adresáø
del jméno
  mazání souboru/souborù
cat jméno
  vypíše obsah souboru na terminál
write jméno
  vytvoøí soubor, pokud neexistuje a èeká na vstup z klávesnice. * = konec dat
append jméno
  èeká na vstup z klávesnice a pøipojuje je na konec souboru. * = konec dat

Zprovoznìní aplikace
======================

1. pøipojte pevnı disk kabelem (popis naleznete v dokumentu `DOCext_ide`_ ), nastavte jej jako MASTER
2. v souboru ``fpga\top_level.vhd`` zvolte pro ide_p vhodnou architekturu (podle zapojení konektoru)
3. pøelote aplikaci
4. naprogramujte MCU a FPGA a spuste terminálovı program
5. po spuštìní je nutné zaèít pøíkazem ``init``
6. pro manipulaci s diskem pouívejte funkce ``mkdir``, ``cd``, ``del``, ``cat``, ``write``, ``append`` a ``close``
