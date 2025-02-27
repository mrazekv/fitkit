.. article:: apps_vga_texture
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>; Ladislav �apka
    :updated: 20090326

    Aplikace demonstruj�c� zobrazen� textur s mo�nost� pohybu jednou texturou pomoc� kl�vesnice. 
    Textury se na��taj� z FLASH pam�ti FITkitu do BRAM v FPGA, odkud prob�h� zobrazov�n�.

========================
Zobrazov�n� textur
========================

Popis aplikace
=================

.. image:: vga_text_img.png
   :align: right

�kolem t�to aplikace je vyu�it� grafick�ho portu pro zobrazov�n� textur (opakuj�c� se vzory). Krom� zobrazov�n� textur obsahuje FPGa logiku, kter� umo��uje pohybovat obr�zkem po texturovan�m pozad�. Zobrazov�n� pohybuj�c�ho se obr�zku ��d� FPGA pomoc� n�kolika kompar�tor�, kter� detekuj� spr�vnou pozici. 

Textury a obr�zek jsou ulo�eny v pam�ti FLASH a pomoc� mikrokontroleru se p�i startu na��taj� do blokov�ch pam�t� RAM (d�le jen BRAM), kter� jsou sou��st� FPGA. Jedna BRAM slou�� k uchov�n� indexu textury (dr�� tedy obsah cel�ho obrazu) a dal�� t�i pro uchov�n� jednotliv�ch RGB kan�l� textur. Mikrokontroler umo��uje aktualizaci textur ve FLASH p�es termin�l (lze nahr�t vlastn� textury) a prov�d� obsluhu posunuj�c�ho obr�zku pomoc� na z�klad� stisku p��slu�n�ch kl�ves kl�vesnice.

Zdrojov� k�dy pro mikrokontroler a FPGA je mo�n� shl�dnout `zde <SVN_APP_DIR>`_.

Hardwarov� ��st (konfigurace pro FPGA)
---------------------------------------
Aplikace vyu��v� zobrazen� na port VGA pomoc� �adi�e VGA. K ulo�en� zobrazovan�ch dat (textur) jsou vyu�ity 3 bloky pam�ti BRAM, kde v ka�d�m bloku je ulo�ena jedna barva z kombinace RGB. Pro ulo�en� pozic textur pozad� slou�� �tvrt� BRAM. O napln�n� t�chto BRAM  p��slu�n�mi daty se star� MCU, kter� p�es adresov� dekod�r SPI ukl�d� do v�ech �ty� BRAM po�adovan� data. Dostupn� pam�ti BRAM maj� dva nez�visl� porty. K jednomu z port� je p�ipojen dekod�r SPI, k druh�mu portu v�ech blok� BRAM je p�ipojen �adi� VGA ��zen� frekvenc� 25 MHz po�adovanou pro grafick� �e�im s rozli�en�m 640x480 obrazov�ch bod� (pixel�) a obnovovac� frekvenc� 60 Hz. V aplikaci jsou vyu�ity 3 SPI dekod�ry. Prvn� je p�ipojen na 3 bloky pam�ti BRAM s obrazov�mi daty, druh� SPI dekod�r je p�ipojen k BRAM, ve kter�m je ulo�ena texturovac� maska. Posledn� SPI dekod�r je vyu�it pro manu�ln� ovl�d�n�, tj. aktu�ln� data o stavu kl�vesnice (LINK) dostupn� p��mo na FITkitu jsou odesl�na do MCU a sou�asn� je z MCU odesl�na aktu�ln� pozice pohybliv�ho obr�zku. FPGA potom zaji��uje na ��dost VGA �adi�e grafick� data, kter� jsou v�dy dostupn� v uveden�ch BRAM a ��d�, kter� typ texturovac�ho obr�zku se m� kreslit, p��padn� zajist� i data pohybliv�ho obr�zku. Zapojen� jednotliv�ch komponent �adi�� v FPGA spole�n� s vazbou na MCU a v�stupn� port ukazuje n�sleduj�c� obr�zek .

.. figure:: vga_text_sch.png

   Blokov� sch�ma aplikace

Nastaven� SPI adresov�ch dekod�r� je voleno dle po�adavk� na data. Adresov� dekod�r SPI, kter� slou�� k aktualizaci RGB dat v BRAM mus� pokr�t 9 barevn�ch bit�, ze kter�ch lze na FITkitu tvo�it VGA barvy. Pro danou velikost BRAM byla zvolena velikost obr�zku 64x64 pixel� a ka�d� BRAM obsahuje 3 bity jedn� barvy RGB. Obr�zek je d�le rozd�len na 3 texturov� obr�zky a jeden pohybliv�, v�echny o velikosti 32x32 pixel�. Adresov� dekod�r SPI spl�uj�c� tuto �lohu m� nastaveno 16 datov�ch bit� a 12 adresov�ch.

Druh� adresov� dekod�r SPI p�ipojen� ke �tvrt�mu bloku BRAM s daty, kter� definuj� rozlo�en� texturovac�ch obr�zk�, mus� pokr�t 3 typy textur a slo�en�m textur mus� b�t vypln�no rozli�en� 640x480 pixel�. Pro zvolen� �adi� dosta�uj� 2 bity na texturu, kter� definuj� index textury 0 a� 2. P�i rozli�en� 640x480 vypln�n� texturou o velikosti 32x32 je nutn� 20x15 texturov�ch obr�zk�. Pro danou aplikaci tedy byla zvolena velikost 32x32 index� textur.

Posledn� �adi� mus� b�t schopn� p�edat data o stavu kl�vesnice a sou�asn� pozici pohybliv�ho obr�zku. Pro stav dostupn� kl�vesnice s 16ti tla��tky je nutno vyu��t 16 bit�, obsahuj�c�ch stav tla��tka. Pro pozici s nutnost� pokr�t pozici a� 624 je po�adov�no 10 bit� jak pro X tak pro Y. Proto m� posledn� SPI adresov� dekod�r 32 (24) datov�ch bit�. Adresov�n� v tomto p��pad� nen� vyu�ito.


Softwarov� ��st (firmware)
----------------------------

Program pro MCU mus� v t�to aplikaci zastat n�kolik �loh, kter�mi jsou ukl�d�n� dat do pam�ti flash, aktualizaci dat z flash do BRAM v FPGA a pohyb kl�vesnic�. Ulo�en� dat do pam�ti prob�h� ve dvou kroc�ch. Prvn� krok je ulo�en� obr�zku a druh� texturovac� maska. K ulo�en� obr�zku slou�� p��kaz FLASH W TEX, po kter�m je nutn� zadat soubor obsahuj�c� p��slu�n� data. Podobn� je tomu pro masku, p��kaz FLASH W MASK. Druh� ��st je aktualizace dat z pam�ti flash do BRAM v FPGA. Toto se d�je automaticky po spu�t�n� FITkitu a nebo vynucen� p�es termin�l p��kazem UPDATE TEX. Posledn� �lohou MCU je vyhodnocov�n� dat z kl�vesnice a zm�na pozice pohybliv�ho obr�zku. Vyhodnocen� kl�vesnice se prov�d� v�dy s ur�itou prodlevou tak, aby se obr�zek nepohyboval p��li� rychle.


**Struktura vstupn�ch dat**

Vstupn� data, kter� tato aplikace vy�aduje, mus� m�t ur�itou strukturu. Proto�e FITkit obsahuje jen 3 bitov� D/A p�evodn�k p�ed v�stupn�m portem VGA, je zbyte�n� vyu��vat v�ce ne� 3 bity. Proto mus� b�t vstupn� soubor s texturou reprezentovan� jako 9 bitov� RGB (pro ka�dou barvu 3 bity). Pro jednoduchost zpracov�n� p�i p��jmu dat z termin�lu do MCU je ka�d� pixel vstupn�ho obrazu zarovn�n na 16 bit�, jeden pixel tedy tvo�� 2 byty. Velikost obr�zku obsahuj�c�ho 4 podobr�zky je, jak je v��e pops�no, 64x64 pixel�. Cel� soubor m� tedy v�dy velikost 8192 byt�. Pozice t�chto dat v pam�ti flash je od adresy 2000h. S druhou ��st� vstupn�ch dat, kterou je texturovac� maska, je situace podobn�. Jak je pops�no v��e, texturovac� data se skl�daj� z index� textur 0-2 a maj� velikost 32x32. Tato data v pam�ti flash navazuj� na texturov� data a jsou tedy bezprost�edn� za textorov�mi daty ulo�ena, co� p�edzna�uje po��te�n� adresu 4000h. Ve vstupn�m souboru jsou data pro jednoduchost zarovn�na v�dy na cel� byty a velikost souboru je tedy v�dy 1024 byt�. V obou dvou p��padech jsou data v souboru zaps�ny po ��dc�ch za sebou.

Aby mohla aplikace spr�vn� pracovat, mus� m�t k dispozici alespo� n�jak� data. Demonstra�n� data (tzn. masku a textury) je mo�n� naj�t v SVN FITkitu v adres��i `data <SVN_APP_DIR/data>`_.

Zprovozn�n� aplikace
========================
 1. p�elo�te aplikaci 

 2. aktivujte propojku J6 pro povolen� periferi� PC
  
 3. naprogramujte MCU a FPGA a spus�te termin�lov� program

 4. pomoc� vhodn�ho termin�lu nahrajte do FLASH texturu a masku z adres��e ``data``
 
    * ``FLASH W TEX`` - inicializuje odes�l�n� souboru s texturami do pam�ti FLASH
    * ``FLASH W MASK`` - inicializuje odes�l�n� souboru s maskou pozad� do pam�ti FLASH
 
 5. nahrajte textury FPGA

    * ``UPDATE TEX`` - aktualizuje textury v FPGA z pam�ti FLASH

    Nyn� by se m�l na displeji zobrazit obr�zek a pomoc� kl�vesnice je mo�n� s n�m pohybovat.
