.. article:: apps_games_sokoban
    :author: Ji�� �ubr <xsubrj00 AT stud.fit.vutbr.cz>
    :updated: 20090615
    
    Klasick� logick� hra Sokoban jej� hrac� plocha je zobrazov�na na monitoru, kter� je p�ipojen k rozhran� VGA.

========================
Hra Sokoban
========================

Popis aplikace
=================

.. image:: sokoban.png
   :align: right

C�lem logick� hry Sokoban je p�esunout postupn� v�echny bedny um�st�n� na hrac� plo�e na definovanou c�lovou plochu. Vykreslov�n� o kter� se star� FPGA je zalo�eno na aplikaci `zobrazuj�c� textury <DOCapps_vga_texture>`_ a d�le na aplikaci umo��uj�c� `zobrazit text <DOCapps_vga_textmode>`_ (pou�ito pro menu). 
Textury, font a stejn� tak i mapy jednotliv�ch level� jsou ulo�eny v pam�ti FLASH, ze kter� se pot� na��taj�.
Hra umo��uje pomoc� textov�ho menu zvolit level, a zobrazit statistiky. Ty se ukl�daj� do pam�ti FLASH a jsou tak p��stupn� i po odpojen� nap�jec�ho nap�t�.


Pr�b�h hry je zobrazov�n na monitoru p�ipojen�m k portu VGA.
Na LCD displeji FITkitu je mo�no naj�t dal�� informace.
Hra se ovl�d� pomoc� kl�vesnice um�st�n� na FITkitu.

Zdrojov� k�dy aplikace je mo�n� nal�zt `zde <SVN_APP_DIR>`_.


Pou�it�
=================

Po prvn�m naprogramov�n� FITkitu aplikac� Sokoban je zapot�eb� do pam�ti FLASH nahr�t bin�rn� data (font, textury, mapu level�). 
K ovl�d�n� lze pou��t kl�vesnici FITkitu. Pohyb do svisl�ch a vodorovn�ch sm�r� umo��uj� kl�vesy 2 (nahoru), 4 (doleva), 6 (doprava) a 8 (dol�).

Po zad�n� p��kazu RESTART do termin�lu se provede restart hry.

Levely - soubor games.skb
=============================
TBD

Zprovozn�n� aplikace
========================
 1. p�elo�te aplikaci 

    ::

      fcmake && make 

 2. aktivujte propojku J6 pro povolen� periferi� PC
  
 3. naprogramujte MCU a FPGA

    ::

      make load

 6. spus�te termin�lov� program QDevKit a pokud jste tak doposud neu�inili, nahrajte do FLASH pam�ti bin�rn� data, 
    kter� jsou um�st�na v adres��i `file:data`. Postupn� nahrajte font (soubor: `file:font.fnt`), textury (soubor: `file:texture.bin`) a jednotliv� levely Sokobanu (soubor: `file:games.skb`).

    ::
      
      flash w font
      update font

      flash w tex
      update tex

      flash w games

      restart

 7. nyn� je mo�n� aplikaci ovl�dat pomoc� kl�vesnice na FITkitu
      
