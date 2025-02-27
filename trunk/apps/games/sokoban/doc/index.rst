.. article:: apps_games_sokoban
    :author: Jiøí Šubr <xsubrj00 AT stud.fit.vutbr.cz>
    :updated: 20090615
    
    Klasická logická hra Sokoban její hrací plocha je zobrazována na monitoru, kterı je pøipojen k rozhraní VGA.

========================
Hra Sokoban
========================

Popis aplikace
=================

.. image:: sokoban.png
   :align: right

Cílem logické hry Sokoban je pøesunout postupnì všechny bedny umístìné na hrací ploše na definovanou cílovou plochu. Vykreslování o které se stará FPGA je zaloeno na aplikaci `zobrazující textury <DOCapps_vga_texture>`_ a dále na aplikaci umoòující `zobrazit text <DOCapps_vga_textmode>`_ (pouito pro menu). 
Textury, font a stejnì tak i mapy jednotlivıch levelù jsou uloeny v pamìti FLASH, ze které se poté naèítají.
Hra umoòuje pomocí textového menu zvolit level, a zobrazit statistiky. Ty se ukládají do pamìti FLASH a jsou tak pøístupné i po odpojení napájecího napìtí.


Prùbìh hry je zobrazován na monitoru pøipojeném k portu VGA.
Na LCD displeji FITkitu je mono najít další informace.
Hra se ovládá pomocí klávesnice umístìné na FITkitu.

Zdrojové kódy aplikace je moné nalézt `zde <SVN_APP_DIR>`_.


Pouití
=================

Po prvním naprogramování FITkitu aplikací Sokoban je zapotøebí do pamìti FLASH nahrát binární data (font, textury, mapu levelù). 
K ovládání lze pouít klávesnici FITkitu. Pohyb do svislıch a vodorovnıch smìrù umoòují klávesy 2 (nahoru), 4 (doleva), 6 (doprava) a 8 (dolù).

Po zadání pøíkazu RESTART do terminálu se provede restart hry.

Levely - soubor games.skb
=============================
TBD

Zprovoznìní aplikace
========================
 1. pøelote aplikaci 

    ::

      fcmake && make 

 2. aktivujte propojku J6 pro povolení periferií PC
  
 3. naprogramujte MCU a FPGA

    ::

      make load

 6. spuste terminálovı program QDevKit a pokud jste tak doposud neuèinili, nahrajte do FLASH pamìti binární data, 
    která jsou umístìna v adresáøi `file:data`. Postupnì nahrajte font (soubor: `file:font.fnt`), textury (soubor: `file:texture.bin`) a jednotlivé levely Sokobanu (soubor: `file:games.skb`).

    ::
      
      flash w font
      update font

      flash w tex
      update tex

      flash w games

      restart

 7. nyní je moné aplikaci ovládat pomocí klávesnice na FITkitu
      
