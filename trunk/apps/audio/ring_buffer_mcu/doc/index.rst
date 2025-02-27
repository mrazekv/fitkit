.. article:: apps_audio_ring_buffer_mcu
    :author: Karel Slan� <slany AT fit.vutbr.cz>
    :updated: 14.10.2009

    Pr�ce s digit�ln�m zvukem kodeku TLV320AIC23B v MCU

===================================
Digit�ln� zvuk TLV320AIC23B v MCU
===================================
.. contents:: Obsah
   :depth: 1

Popis aplikace
===============

V FPGA je z BRAM vytvo�en kruhov� buffer o kapacit� 512 32-bitov�ch polo�ek. Ka�d� polo�ka se skl�d� z dvou 16-bitov�ch vzork�, pro lev� a prav� kan�l jeden.
Pou�it� BRAM m� dva porty, kter� zp��stup�uj� jej� obsah. Port A slou�� k z�pisu/�ten� dat do/z kodeku, k portu B je p�ipojen �adi� SPI p�ipojen� k MCU.
Tento SPI �adi� slou�� �ist� k p�enosu zvukov�ch dat mezi FPGA a MCU a tvo�� tak samostatn� p�enosov� kan�l s p�enosovou rychlost� 14MHz v ka�d�m sm�ru.
MCU pak realizuje �ten� zvukov�ch dat a jejich zp�tn� z�pis. Cel� aplikace se chov� jako propojen� digit�ln�ho v�stupu s digit�ln�m vstupem kodeku p�es MCU.

Obslu�n� aplikace digitalizuje vstupn� sign�l z linkov�ho vstupu JP6 a p�ehr�v� jej na v�stupu JP7.

.. figure:: wire_mcu.png

   Sch�ma propojen� komponent.

Na obr�zku `wire_mcu.png`_ je zn�zorn�no blokov� sch�ma zapojen� komponent. Blo�ky ``codec reader`` a ``codec writer`` jsou realizov�ny jako samostatn� generick� komponenty ``aic23b_dsp_read`` a ``aic23b_dsp_write``, kter� realizuj� p�evod sign�lu pro p�enos do kodeku.
Dohromady s ``BRAM`` tvo�� komponentu ``aic23b_dsp_ringbuffer`` jej� VHDL popis naleznete v souboru `fpga/ctrls/audio/aic23b_dsp_ringbuffer.vhd <SVN_DIR/trunk/fpga/ctrls/audio/aic23b_dsp_ringbuffer.vhd>`_.
Blok ``fast SPI`` realizuje �adi� druh�ho SPI pro p�enos zvuku. VHDL popis komponenty naleznete v souboru `fpga/spi_controller.vhd <SVN_APP_DIR/fpga/spi_controller.vhd>`_.
Bloky ``BRAM CU`` jsou realizov�ny jako procesy. Slou�� k v�po�tu adresy pro z�pis a �ten� z BRAM. Jsou ��zeny komponentami, kter� realizuj� komunika�n� rozhran� pro kodek a MCU.


Ovl�d�n� aplikace
------------------

Kodek TLV320AIC23B se d� rozd�lit na dv� ��sti - analogovou (zesilova�e a mixer) a digit�ln� (A/D a D/A sigma delta p�evodn�k, digit�ln� s�riov� rozhran�). Analogov� ��st umo��uje obej�t (bypass) digit�ln� ��st.
Aktivace digit�ln�ho rozhran� se v aplikaci provede p��kazem ``DSP_ON``, p��kazem ``BYPASS_DSP`` deaktivujete digit�ln� rozhran� a zapne se bypass. Pokud nesly��te zvuk ani po zad�n� p��kazu ``BYPASS_DSP`` ujist�te se, zda m�te spr�vn� p�ipojen linkov� vstup.

P��kazem ``DSP_ON`` se skrze konfigura�n� SPI rozhran� kodeku nastav� konfigura�n� registry, ��m� se aktivuj� A/D D/A p�evodn�ky, nastav� komunika�n� protokol na DSP re�im, vzorkovac� frekvence a bitov� ���ka vzork�.

P��kaz ``BYPASS_DSP`` deaktivuje digit�ln� p�evodn�ky a aktivuje analogov� bypass, kter�m se propoj� analogov� vstup s analogov�m v�stupem.

V obou p�edchoz�ch p��padech, je aktivn� v�stup JP7.

P��kazem ``MUTE`` se v obou re�imech sn�� hlasitost v�stupu na minimum. P��kaz ``RESTORE`` obnov� p�vodn� hlasitost v�stupu.


Rozhran� SPI pro p�enos zvuku
=====================================

MCU je v t�to aplikaci p�ipojeno k FPGA pomoc� dvou SPI rozhran�. Prvn� SPI je klasick�, ur�en� pro komunikaci s SPI �adi�em. Je definov�no v knihovn� libfitkit.
Druh� SPI rozhran� slou�� pouze k p�enosu digit�ln�ho zvuku. Funkce pro MCU naleznete v souborech `mcu/fspi.h <SVN_APP_DIR/mcu/fspi.h>`_ a `mcu/fspi.c <SVN_APP_DIR/mcu/fspi.c>`_.
Toto rozhran� pracuje na 14MHz.

Zprovozn�n� aplikace
========================
1. P�ed spu�t�n�m kodeku se ujist�te, �e propojky J3, J4 jsou aktivn� a J2 je neaktivn�.

2. Do konektoru JP7 zapojte sluch�tka a do JP6 zdroj sign�lu (nap�. MP3 p�ehr�va�).

3. P�elo�te aplikaci.

4. Naprogramujte MCU a FPGA a spus�te termin�lov� program.

   Nyn� byste m�li ve sluch�tk�ch sly�et zvuk (vzorkovan� na 8kHz) p�ich�zej�c� do linkov�ho vstupu JP6.
