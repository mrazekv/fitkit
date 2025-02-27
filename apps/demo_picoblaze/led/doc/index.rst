.. article:: apps_demo_picoblaze_led
   :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
   :updated: 11.10.2009

   Jednoduch� aplikace demonstruj�c� ��zen� LED diody pomoc� procesoru PicoBlaze implementovan�ho uvnit� FPGA.

==================================================
Blik�n� LED diodou (ralizace zpo��ovac� smy�ky)
==================================================

Popis aplikace
=================
Tato jednoduch� aplikace demonstruje ��zen� LED diody pomoc� 8-bitov�ho procesoru Picoblaze od firmy XILINX, kter� je implementov�n uvnit� FPGA jako komponenta PicoCPU. 
PicoBlaze je ��zen programem, kter� pravideln� ka�dou sekundu inkrementuje hodnotu jednoho registru a tuto hodnotu zapisuje na v�stupn� port procesoru, kde je p�ipojena �lut� LED dioda.
Proto�e je dioda p�ipojena na nejni��� bit, blik� v intervalu jedna sekunda.


N�vrh aplikace (FPGA)
=======================

V t�to aplikaci jsou pou�ity dva adresov� dekod�ry SPI_ADC (viz `DOCfpga_interconect`_) a komponenta `PicoCPU. <DOCfpga_picocpu>`_ Komponenta PicoCPU je propojena s jedn�m z adresn�ch dekod�r�, d�ky n�mu� je umo�n�n p��stup k pam�ti programu. Druh� adresn� dekod�r je vyu�it pro ��zen� procesoru - umo��uje jeho spu�t�n� a zastaven�. Z toho d�vodu je za t�mto dekod�rem p�ipojen registr, kter� dr�� ulo�enu zapsanou hodnotu.

.. figure:: app_demo_pled.png

   Propojen� komponent uvnit� FPGA

Proto�e procesor PicoBlaze pou��v� z d�vodu efektivn�ho vyu�it� pam�t� BlockRAM 18-bitov� instrukce, je pro jednoduchost pou�ita v dekod�ru 24 bitov� datov� ���ka slova. Adresace pam�ti prob�h� prost�ednictv�m 10-bitov�ho adresn�ho sign�lu. Nastaven� generick�ch parametr� pro SPI dekod�r slou��c�ch pro p�enos instrukc� programu:

  * `generic:ADDR_WIDTH` = 16 (���ka adresy)
  * `generic:DATA_WIDTH` = 24 (���ka dat)
  * `generic:ADDR_OUT_WIDTH` = 10 (���ka v�stupn� adresy)
  * `generic:BASE_ADDR` = 16#F800# (b�zov� adresa SPI dekod�ru).

Druh�h� adresn� dekod�r slou�� pouze k p�enosu jednobitov� informace, kter� povoluje/zakazuje chod procesoru. Instrukce pro procesor lze nahr�t do pam�ti pomoc� termin�lu. Uk�zkov� k�d je mo�n� naj�t v SVN ve slo�ce `data <SVN_APP_DIR/data>`_. Zdrojov� k�d je nutn� p�elo�it p�eklada�em ``KCPSM3``, kter� je sou��st� bal�ku PicoBlaze, jeho� popis m��ete nal�zt na str�nce `DOCfpga_picocpu`_.

N�vrh aplikace (Picoblaze - counter.psm)
==========================================
Program pro procesor PicoBlaze je naps�n v Assembleru a obsahuje dv� rutiny (``main`` a ``delay``). Hlavn� programov� smy�ka zapisuje na port hodnotu registru ``r7`` a tuto hodnotu inkrementuje. 
�kolem rutiny ``delay`` je realizovat co nejp�esn�ji zpo�d�n� 1 sekundu pomoc� smy�ek. Zde vych�z�me ze znalosti frekvence, na kter� je PicoBlaze provozov�n a d�le ze znalosti doby trv�n� jedn� instrukce, co� je dle datasheetu dva hodinov� cykly.

N�vrh aplikace (MCU)
=======================
P�edt�m, ne� procesor PicoBlaze spust�me, je zapot�eb� nahr�t do instruk�n� pam�ti program. Program lze nahr�t pomoc� termin�lu. 

Program pro MCU vych�z� �e�� p��jem a zpracov�n� souboru `file:.hex`, kter� je v�stupem p�eklada�e assembleru KCPSM3. Tento soubor je mikrokontrolerem p�ijat a ulo�en do pam�ti FLASH, aby nebylo nutn� v�dy po spu�t�n� FITkitu program nahr�vat znovu. K z�pisu do FLASH slou�� p��kaz ``FLASH W HEX``. Soubor `file:.hex`  obsahuje v�dy 1024 instrukc�. Na jednom ��dku je ulo�ena pr�v� jedna instrukce. Ka�d� instrukce je ulo�ena jako hexadecim�ln� ��slo. 

Po spu�t�n�/resetu FITkitu je program pro PicoBlaze automaticky odesl�n z pam�ti FLASH prost�ednictv�m sb�rnice SPI do FPGA, kde ho p�ij�m� adresn� dekod�r a zapisuje do BlockRAM pam�ti nach�zej�c� se uvnit� PicoCPU. Aktualizaci programu z pam�ti FLASH je mo�n� prov�st pomoc� p��kazu ``CPU UPDATE``. Pro povolen� p��padn� zak�z�n� vykon�v�n� instrukc� procesorem PicoBlaze je mo�n� vyu��t p��kazy ``CPU START`` a ``CPU STOP``.

Zprovozn�n� aplikace
========================
   1. Ze str�nek XILINX st�hn�te implementaci PicoBlaze v jazyku VHDL. Dle pokyn� uveden�ch v `DOCfpga_picocpu`_ zprovozn�te komponentu PicoCPU.

   2. P�elo�te aplikaci

   3. Spus�te termin�lov� program QDevKit a naprogramujte MCU a FPGA

   4. Zahajte komunikaci s FITkitem a pomoc� p��kazu ``FLASH W HEX`` nahrajte program pro procesor PicoBlaze um�st�n� v `file:data/counter.hex` do pam�ti FLASH um�st�n� na FITkitu.

      K�d pro PicoBlaze lze v p��pad� pot�eby ze zdrojov�ho souboru p�elo�it pomoc� p��kazu ``kcpsm3.exe counter.psm``.

   5. Pomoc� p��kazu ``CPU UPDATE`` inicializujte pam� programu uvnit� FPGA a povolte �innost procesoru.

   6. Procesor za�ne vykon�vat instrukce ��m� bude blikat dioda D4
