.. article:: apps_communication_ide
   :author: Stanislav Sigmund <xsigmu02 AT fit.vutbr.cz>
   :updated: 12.6.2009

   Aplikace umo��uj�c� p�istupovat k pevn�mu disku, proch�zet a m�nit adres��ovou strukturu a soubory

===============================
Obsluha IDE disku
===============================

Popis aplikace
=================
Aplikace pou��v� IDE �adi� s DMA p�enosem, umo��uje proch�zet adres��ovou strukturu, vytv��et/mazat adres��e a prohl�et/modifikovat obsah soubor�.

Podporovan� p��kazy:
---------------------
init
  inicializuje disk a na�te 1. partition na disku
close
  ulo�� �daje o ovln�m m�st�, vhodn� prov�st po z�pisu na disk p�ed ukon�en�m aplikace
dir/dirp[ jm�no]
  zobraz� seznam v�ech polo�ek, p��padn� jen polo�ky zadan�ho jm�na (lze pou��t hv�zdi�kovou konvenci). verze dirp zobrazuje pouze jm�na
mkdir jm�no
  vytvo�� adres��
cd
  p�ejde na adres��, pracuje pouze o jednu �rove�, \ = ko�enov� adres��
del jm�no
  maz�n� souboru/soubor�
cat jm�no
  vyp�e obsah souboru na termin�l
write jm�no
  vytvo�� soubor, pokud neexistuje a �ek� na vstup z kl�vesnice. * = konec dat
append jm�no
  �ek� na vstup z kl�vesnice a p�ipojuje je na konec souboru. * = konec dat

Zprovozn�n� aplikace
======================

1. p�ipojte pevn� disk kabelem (popis naleznete v dokumentu `DOCext_ide`_ ), nastavte jej jako MASTER
2. v souboru ``fpga\top_level.vhd`` zvolte pro ide_p vhodnou architekturu (podle zapojen� konektoru)
3. p�elo�te aplikaci
4. naprogramujte MCU a FPGA a spus�te termin�lov� program
5. po spu�t�n� je nutn� za��t p��kazem ``init``
6. pro manipulaci s diskem pou��vejte funkce ``mkdir``, ``cd``, ``del``, ``cat``, ``write``, ``append`` a ``close``
