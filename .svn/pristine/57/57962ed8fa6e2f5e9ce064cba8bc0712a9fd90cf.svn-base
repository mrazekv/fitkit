.. article:: apps_vga_texture
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>; Ladislav Èapka
    :updated: 20090326

    Aplikace demonstrující zobrazení textur s moností pohybu jednou texturou pomocí klávesnice. 
    Textury se naèítají z FLASH pamìti FITkitu do BRAM v FPGA, odkud probíhá zobrazování.

========================
Zobrazování textur
========================

Popis aplikace
=================

.. image:: vga_text_img.png
   :align: right

Úkolem této aplikace je vyuití grafického portu pro zobrazování textur (opakující se vzory). Kromì zobrazování textur obsahuje FPGa logiku, která umoòuje pohybovat obrázkem po texturovaném pozadí. Zobrazování pohybujícího se obrázku øídí FPGA pomocí nìkolika komparátorù, které detekují správnou pozici. 

Textury a obrázek jsou uloeny v pamìti FLASH a pomocí mikrokontroleru se pøi startu naèítají do blokovıch pamìtí RAM (dále jen BRAM), které jsou souèástí FPGA. Jedna BRAM slouí k uchování indexu textury (drí tedy obsah celého obrazu) a další tøi pro uchování jednotlivıch RGB kanálù textur. Mikrokontroler umoòuje aktualizaci textur ve FLASH pøes terminál (lze nahrát vlastní textury) a provádí obsluhu posunujícího obrázku pomocí na základì stisku pøíslušnıch kláves klávesnice.

Zdrojové kódy pro mikrokontroler a FPGA je moné shlédnout `zde <SVN_APP_DIR>`_.

Hardwarová èást (konfigurace pro FPGA)
---------------------------------------
Aplikace vyuívá zobrazení na port VGA pomocí øadièe VGA. K uloení zobrazovanıch dat (textur) jsou vyuity 3 bloky pamìti BRAM, kde v kadém bloku je uloena jedna barva z kombinace RGB. Pro uloení pozic textur pozadí slouí ètvrtá BRAM. O naplnìní tìchto BRAM  pøíslušnımi daty se stará MCU, kterı pøes adresovı dekodér SPI ukládá do všech ètyø BRAM poadovaná data. Dostupné pamìti BRAM mají dva nezávislé porty. K jednomu z portù je pøipojen dekodér SPI, k druhému portu všech blokù BRAM je pøipojen øadiè VGA øízenı frekvencí 25 MHz poadovanou pro grafickı øeim s rozlišením 640x480 obrazovıch bodù (pixelù) a obnovovací frekvencí 60 Hz. V aplikaci jsou vyuity 3 SPI dekodéry. První je pøipojen na 3 bloky pamìti BRAM s obrazovımi daty, druhı SPI dekodér je pøipojen k BRAM, ve kterém je uloena texturovací maska. Poslední SPI dekodér je vyuit pro manuální ovládání, tj. aktuální data o stavu klávesnice (LINK) dostupné pøímo na FITkitu jsou odeslána do MCU a souèasnì je z MCU odeslána aktuální pozice pohyblivého obrázku. FPGA potom zajišuje na ádost VGA øadièe grafická data, která jsou vdy dostupná v uvedenıch BRAM a øídí, kterı typ texturovacího obrázku se má kreslit, pøípadnì zajistí i data pohyblivého obrázku. Zapojení jednotlivıch komponent øadièù v FPGA spoleènì s vazbou na MCU a vıstupní port ukazuje následující obrázek .

.. figure:: vga_text_sch.png

   Blokové schéma aplikace

Nastavení SPI adresovıch dekodérù je voleno dle poadavkù na data. Adresovı dekodér SPI, kterı slouí k aktualizaci RGB dat v BRAM musí pokrıt 9 barevnıch bitù, ze kterıch lze na FITkitu tvoøit VGA barvy. Pro danou velikost BRAM byla zvolena velikost obrázku 64x64 pixelù a kadá BRAM obsahuje 3 bity jedné barvy RGB. Obrázek je dále rozdìlen na 3 texturové obrázky a jeden pohyblivı, všechny o velikosti 32x32 pixelù. Adresovı dekodér SPI splòující tuto úlohu má nastaveno 16 datovıch bitù a 12 adresovıch.

Druhı adresovı dekodér SPI pøipojenı ke ètvrtému bloku BRAM s daty, která definují rozloení texturovacích obrázkù, musí pokrıt 3 typy textur a sloením textur musí bıt vyplnìno rozlišení 640x480 pixelù. Pro zvolenı øadiè dostaèují 2 bity na texturu, které definují index textury 0 a 2. Pøi rozlišení 640x480 vyplnìné texturou o velikosti 32x32 je nutné 20x15 texturovıch obrázkù. Pro danou aplikaci tedy byla zvolena velikost 32x32 indexù textur.

Poslední øadiè musí bıt schopnı pøedat data o stavu klávesnice a souèasnì pozici pohyblivého obrázku. Pro stav dostupné klávesnice s 16ti tlaèítky je nutno vyuít 16 bitù, obsahujících stav tlaèítka. Pro pozici s nutností pokrıt pozici a 624 je poadováno 10 bitù jak pro X tak pro Y. Proto má poslední SPI adresovı dekodér 32 (24) datovıch bitù. Adresování v tomto pøípadì není vyuito.


Softwarová èást (firmware)
----------------------------

Program pro MCU musí v této aplikaci zastat nìkolik úloh, kterımi jsou ukládání dat do pamìti flash, aktualizaci dat z flash do BRAM v FPGA a pohyb klávesnicí. Uloení dat do pamìti probíhá ve dvou krocích. První krok je uloení obrázku a druhı texturovací maska. K uloení obrázku slouí pøíkaz FLASH W TEX, po kterém je nutné zadat soubor obsahující pøíslušná data. Podobnì je tomu pro masku, pøíkaz FLASH W MASK. Druhá èást je aktualizace dat z pamìti flash do BRAM v FPGA. Toto se dìje automaticky po spuštìní FITkitu a nebo vynucenì pøes terminál pøíkazem UPDATE TEX. Poslední úlohou MCU je vyhodnocování dat z klávesnice a zmìna pozice pohyblivého obrázku. Vyhodnocení klávesnice se provádí vdy s urèitou prodlevou tak, aby se obrázek nepohyboval pøíliš rychle.


**Struktura vstupních dat**

Vstupní data, která tato aplikace vyaduje, musí mít urèitou strukturu. Protoe FITkit obsahuje jen 3 bitovı D/A pøevodník pøed vıstupním portem VGA, je zbyteèné vyuívat více ne 3 bity. Proto musí bıt vstupní soubor s texturou reprezentovanı jako 9 bitové RGB (pro kadou barvu 3 bity). Pro jednoduchost zpracování pøi pøíjmu dat z terminálu do MCU je kadı pixel vstupního obrazu zarovnán na 16 bitù, jeden pixel tedy tvoøí 2 byty. Velikost obrázku obsahujícího 4 podobrázky je, jak je vıše popsáno, 64x64 pixelù. Celı soubor má tedy vdy velikost 8192 bytù. Pozice tìchto dat v pamìti flash je od adresy 2000h. S druhou èástí vstupních dat, kterou je texturovací maska, je situace podobná. Jak je popsáno vıše, texturovací data se skládají z indexù textur 0-2 a mají velikost 32x32. Tato data v pamìti flash navazují na texturová data a jsou tedy bezprostøednì za textorovımi daty uloena, co pøedznaèuje poèáteèní adresu 4000h. Ve vstupním souboru jsou data pro jednoduchost zarovnána vdy na celé byty a velikost souboru je tedy vdy 1024 bytù. V obou dvou pøípadech jsou data v souboru zapsány po øádcích za sebou.

Aby mohla aplikace správnì pracovat, musí mít k dispozici alespoò nìjaká data. Demonstraèní data (tzn. masku a textury) je moné najít v SVN FITkitu v adresáøi `data <SVN_APP_DIR/data>`_.

Zprovoznìní aplikace
========================
 1. pøelote aplikaci 

 2. aktivujte propojku J6 pro povolení periferií PC
  
 3. naprogramujte MCU a FPGA a spuste terminálovı program

 4. pomocí vhodného terminálu nahrajte do FLASH texturu a masku z adresáøe ``data``
 
    * ``FLASH W TEX`` - inicializuje odesílání souboru s texturami do pamìti FLASH
    * ``FLASH W MASK`` - inicializuje odesílání souboru s maskou pozadí do pamìti FLASH
 
 5. nahrajte textury FPGA

    * ``UPDATE TEX`` - aktualizuje textury v FPGA z pamìti FLASH

    Nyní by se mìl na displeji zobrazit obrázek a pomocí klávesnice je moné s ním pohybovat.
