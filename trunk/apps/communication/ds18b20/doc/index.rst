.. article:: apps_communication_ds18b20
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20090404

    Komunikace s teplotn�m �idlem DS18B20 p�ipojen�m ke sb�rnici one-wire.

===============================
M��en� teploty pomoc� DS18B20
===============================

Senzor DS18B20
=================
`DS18B20 <http://pdfserv.maxim-ic.com/en/ds/DS18B20.pdf>`_ je b�n� dostupn� digit�ln� teplom�r, kter� je schopen m��it v rozsahu -55�C a� +125�C s p�esnost� �0.5�C garantovanou v teplotn�m rozsahu -10�C to +85�C. 
Data jsou ze senzoru z�sk�v�na pomoc� jedno-vodi�ov�ho seriov�ho rozhran� (`1-wire interface <http://en.wikipedia.org/wiki/1-Wire>`_) s u�ivatelsky programovatelnou p�esnost� 9 - 12 bit� (tzn. rozli�en� rozli�en� 0.5 - 0.0625�C). 
Pro komunikaci se pou��v� jeden datov� a dva napajec� vodi�e (GND, VDD, DQ). Nap�jec� nap�t� VDD se m��e pohyboat v rozmez� 3.3V a� 5.5V. Mimo to lze senzor provozovat v tzv. parazitn�m re�imu vyu��vaj�c�m pouze dvou vodi��.

.. figure:: ds18b20multi2.png

   Obvod DS18B20 a zp�sob zapojen� v�ce senzor� na sb�rnici

Krom� m��en� teploty obvod disponuje funkc� digit�ln�ho termostatu, jeho� doln� a horn� hranici lze programov� nastavit a ulo�it do EEPROM pam�ti na �ipu. P�i �ten� nam��en� hodnoty jsou pak k dispozici bity informuj�c� o p�ekro�en� naprogramovan�ch rozsah�.

Ka�d� obvod m� ji� z v�roby p�id�leno sv� vlastn� unik�tn� 64-bitov� seriov� ��slo, kter� slou�� k adresaci konkr�tn�ho senzoru. Seriov� ��slo spole�n� s unik�tn�m protokolem identifikace p�ipojen�ch za��zen� ke sb�rnici umo��uje provozovat n�kolik senzor� na jednom datov�m vodi�i. Mimo to nab�z� obvod mo�nost vyu��t nap�ov� sign�l pro p�enos dat, ��m� se celkov� po�et vodi�� nutn� k p�ipojen� senzor� redukuje na dva (GND, VDD/DQ).

Popis aplikace
=================
Aplikace umo��uje sn�mat teplotu z jednoho senzoru DS18B20 p�ipojen�ho k jednomu z pin� FPGA obvodu.
Komunikaci s �idlem a realizaci 1-wire protokolu zaji��uje �adi� uvnit� FPGA. 
Mikrokontroler obsahuje rutiny, kter� jsou schopny znicializovat teplotn� �idlo a zm��it teplotu. Rutiny vyu��vaj� p��kazu �ten� a z�pisu bytu poskytovan�ho FPGA �adi�em.

�adi� v FPGA se skl�d� z kone�n�ho automatu, kter� zaji��uje a) �ten� 8-bitov� informace, b) z�pis 8-bitov� informace, c) inicializaci teplotn�ho sensoru a detekci jeho p�ipojen�. 
Zdrojov� k�d �adi�e lze nal�zt v souboru `one_wire.vhd <SVN_APP_DIR/fpga/one_wire.vhd>`_.

Vlastn� m��en� realizuje rutina ``measuretemp()`` uveden� v souboru `file:main.c`.
Tato funkce vyu��v� n�sleduj�c� jednoduch� rutiny komunikuj�c� s �adi�em v FPGA:
* ``onewire_reset()`` - na��d� �adi�i v FPGA zinicializovat za��zen� (je-li n�jak� p�ipojeno)
* ``onewire_write()`` - �adi� v FPGA po�le p�es 1-wire 8-bitov� slovo
* ``onewire_read()`` - �adi� v FPGA p�e�te z 1-wire 8-bitov� slovo a vr�t� je mikrokontoleru (v rutin� se aktivn� �ek� na dokon�en� operace)

Pomoc� t�chto funkc� jsou zas�l�ny p��kazy a �teny data ze senzoru.
Nejd��ve je zah�jen p�evod teploty (p��kaz 0xCC 0x44), pot� se aktivn� �ek�, a� je p�evod dokon�en a n�sledn� je p�e�tena nam��en� hodnota (p��kaz 0xCC 0xBE). Bli��� informace naleznete v p��slu�n�m datasheetu.

Zdrojov� k�dy aplikace lze nal�zt v `SVN <SVN_APP_DIR>`_.


Zprovozn�n� aplikace
=====================

   1. p�elo�te aplikaci

   2. p�ipojte teplotn� senzor na p�edposledn� pin horn� �ady konektoru JP10 dle n�kresu na obr�zku `ds18b20fitkit.png`_. Datov� vodi� je nutn� opat�it pull-up resistorem o odporu n�kolik kilo ohm�.

      .. figure:: ds18b20fitkit.png

         P�ipojen� teplotn�ho �idla k FITkitu

   3. naprogramujte MCU a FPGA a spus�te termin�lov� program nebo vyt�hn�te propojky J8/J9
      
      Pokud je senzor p�ipojen, m�la by se na displeji zobrazit nam��en� teplota, kter� se aktualizuje ka�dou sekundu.
