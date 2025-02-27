.. article:: apps_communication_serial_echo
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20090404

    Komunikace mezi �adi�em s�riov� linky v FPGA a PC (komunika�n�m kan�lem A obvodu FTDI)

===============================
Seriov� linka v FPGA (echo)
===============================

Popis aplikace
=================

C�lem aplikace je uk�zat, jak je mo�n� uvnit� FPGA vytvo�it �adi� seriov� linky a jak lze pomoc� �adi�e p�en�et data z/do PC.
�adi� je p�ipojen k `USB p�evodn�ku FTDI <DOChw_ftdi>`_, kan�lu A.

.. note:: Jeliko� data v t�to jednoduch� aplikaci putuj� (kv�li zjednodu�en� k�du) z�m�rn� znak po znaku z FPGA do MCU a zp�t, je nutn� m�t na pam�ti s t�m souvisej�c� probl�my. Vzhledem k latenci mezi vyvol�n�m ��dosti o p�eru�en�, vlastn� obsluhou p�eru�en� a vy�ten� p�ijat�ho bytu p�es SPI m��e doch�zet ke ztr�t� p�ijat�ch dat v p��pad�, �e bude odvys�l�no v�ce byt� sou�asn�. Oby�ejn� nen� pot�eba data p�epos�lat p�es MCU a pokud ano, pak je nutn� vytvo�it uvnit� FPGA FIFO pam�, kter� bude shroma��ovat p�ijat� byty.

Zdrojov� k�dy aplikace lze nal�zt v `SVN <SVN_APP_DIR>`_.


Zprovozn�n� aplikace
=====================

1. p�elo�te aplikaci 

   ::

     fcmake && make 


2. pro spr�vnou �innost je nutn� povolit p�eru�en� z FPGA do MCU

   *  FITkit verze 1.x:
       
      propojit pin JP10(5) vedouc� z FPGA s pinem JP9(26) vedouc�m do MCU

   *  FITkitu verze 2.x:
     
      propojit propojku J5, kter� je ur�ena k povolen� p�eru�en�

3. naprogramujte MCU a FPGA a spus�te termin�lov� program (nap�. QDevKit), p��padn� rozpojte propojky J8,J9

    ::

      make load
      make term

4. p�ipojte se pomoc� vhodn�ho termin�lov�ho programu na kan�l A USB p�evodn�ku (bu� p��mo nebo p�es virtu�ln� COM port, jeho� ��slo naleznete ve spr�vci za��zen� - viz dokument `DOCinstall_ft2232c`_).
   Parametry nastavte n�sledovn�: komunika�n� rychlost 57600 Bd, 8 datov�ch bit�, lich� parita, 1 stop bit.

   .. figure:: serialsettings.png

      Hypertermin�l

   P�i ka�d�m stisku kl�vesy v termin�lu p�ipojen�m ke kan�lu A se v tomt� termin�lu zobraz� odpov�daj�c� znak (echo) a dioda D4 zhasne/se rosv�t�. Jste-li p�ipojeni k mikrokontroleru, je ka�d� p�ijat� znak zobrazen tak� v p��slu�n�m termin�lov�m okn� QDevKitu.
