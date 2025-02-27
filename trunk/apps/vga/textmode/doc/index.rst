.. article:: apps_vga_textmode
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20090326

    Aplikace umoòující provozovat monitor pøipojenı ke kitu jako terminál v textovém reimu.

========================
Textovı reim
========================

Popis aplikace
=================

.. image:: vga_textm_img.png
   :align: right

Aplikace provozuje monitor v rozlišení 640x480 bodù a k zobrazení textu je pouita standardní velikost fontu 8x16 bodù (8 bodù na šíøku, 16 bodù na vıšku).
Na obrazovku se tedy vejde celkem 30 øádkù textu o šíøce 80 znakù. Text je moné na obrazovku psát pomocí pøíkazù zadanıch v terminálu.

Jednotlivé znaky fontu jsou v souboru i v pamìti fontu uloeny jako šestnáctice bytù, jednotlivé bity jednoho bytu odpovídají bodùm v jednom øádku znaku.
Znaky jsou uloeny v pamìti FLASH a pomocí mikrokontroleru se pøi startu naèítají do blokové pamìti RAM (dále jen BRAM), která jsou souèástí FPGA. 
Pro jednoduchost font obsahuje pouze 128 znakù.
Jedna BRAM slouí k uchování indexu jednotlivıch znakù (obsahuje tedy informaci o celém obrazu) a další BRAM pamì je vyuita pro uchování jednotlivıch znakù fontu. Indexaèní pamì tedy v závislosti na pozici vykreslovaného bodu ukazuje do pamìti fontu odkud se postupnì zobrazují jednotlivé bity (pixely).

Zprovoznìní aplikace
========================
 1. pøelote aplikaci 

 2. aktivujte propojku J6 pro povolení periferií PC
  
 3. naprogramovujte MCU a FPGA a spuste terminálovı program

 4. pomocí vhodného terminálu nahrajte do FLASH font z adresáøe ``data``. 
    
    ``FLASH W FONT`` - inicializuje odesílání souboru s fontem do pamìti FLASH
    
    ``UPDATE FONT`` - aktualizuje textury v FPGA z pamìti FLASH

 5. text je moné zobrazit zadáním pøíkazu ``VGA TEXT text``
