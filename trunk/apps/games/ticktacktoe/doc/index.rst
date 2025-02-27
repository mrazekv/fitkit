.. article:: apps_games_tictactoe
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20090326
    
    Hra pi�kvorky ur�en� pro jeden/dva kity. Hrac� plocha je zobrazov�na na monitoru, kter� je p�ipojen k rozhran� VGA.

========================
Hra pi�kvorky
========================

Popis aplikace
=================

.. image:: ticktacktoe.png
   :align: right

C�lem aplikace je uk�zat, jak je mo�n� vytvo�it jednoduchou hru pro dva hr��e. Vykreslov�n� ��st o kterou se star� FPGA je zalo�ena na aplikaci `zobrazuj�c� textury <DOCapps_vga_texture>`_. Textury (kole�ko, k��ek, pr�zdn� a pln� �tvere�ek) jsou v�ak v tomto p��pad� po��t�ny v mikrokontroleru a p�eneseny do BRAM pam�t� v FPGA p�ed zah�jen�m hry. 

Hru mohou hr��i hr�t st��dav� na jednom FITkitu a nebo vyu��t propojen� dvou kit�, kdy ka�d� hr�� m��e hr�t na sv�m kitu. P�ed zah�jen�m hry se prov�d� detekce kit� a kity se mus� dohodnoout, kter� bude za��nat jako prvn�.
K tomuto je zapot�eb� zdroj pseudo n�hodn�ch ��sel. V na�em p��pad� byl pou�it teplotn� sensor, kter� je integrov�n v mikrokontroleru, jeho� nep�esnost je v t�to aplikaci v�hodou.

Pr�b�h hry je zobrazov�n na monitoru p�ipojen�m k portu VGA. 
Na LCD displeji FITkitu je mo�no naj�t informace o tom, kter� hr�� je na tahu a p��padn� kter� vyhr�l.

Zdrojov� k�dy aplikace je mo�n� nal�zt `zde <SVN_APP_DIR>`_.


Pou�it�
=================

Ihned po zapnut� se prov�d� detekce, zda-li je p�ipojen druh� kit (ka�d� kit si vygeneruje n�hodn� ��slo a sna�� se domluvit, kdo bude hru ��dit). 
Pokud se poda�� kit detekovat, hraje ka�d� hr�� na sv�m kitu, ke kter�mu je p�ipojen monitor. 
Pokud se nepoda�� detekovat druh� kit, st��daj� se hr��i na jednom kitu.

K ovl�d�n� lze pou��t kl�vesnici FITkitu. Pohyb do svisl�ch a vodorovn�ch sm�r� umo��uj� kl�vesy 2 (nahoru), 4 (doleva), 6 (doprava) a 8 (dol�). Kl�vesa 5 slou�� k polo�en� symbolu - k��ek/kole�ko.

Po zad�n� p��kazu RESTART do termin�lu se provede restart hry.


Zprovozn�n� aplikace
========================
 1. p�elo�te aplikaci 

    ::

      fcmake && make 

 2. Aktivujte propojku J6 pro povolen� periferi� PC
  
 3. Pro spr�vnou �innost je nutn� povolit p�eru�en� z FPGA do MCU

    *  FITkit verze 1.x:
       
       propojit pin JP10(5) vedouc� z FPGA s pinem JP9(26) vedouc�m do MCU

    *  FITkitu verze 2.x:
     
       propojit propojku J5, kter� je ur�ena k povolen� p�eru�en�


 4. Aby fungovala hra na dvou kitech, je nutne spojit n�sleduj�c� piny:
    *  FITkit verze 1.x:
       
       pin JP10(47) prvn�ho kitu s pinem JP10(48) na druh�m kitu a
       
       pin JP10(48) prvn�ho kitu s pinem JP10(47) na druh�m kitu

    *  FITkitu verze 2.x:
      
       pin JP10(49) prvn�ho kitu s pinem JP10(50) na druh�m kitu a
      
       pin JP10(50) prvn�ho kitu s pinem JP10(49) na druh�m kitu

    jinou mo�nost� je pou��t k��en� kabel a konektor RS232, kter� je na kitu

 5. naprogramovat MCU a FPGA

    ::

      make load

 6. spustit termin�lov� program, p��padn� rozpojit propojky J8,J9 a stisknout tla��tka reset na obou kitech

    ::

      make term
