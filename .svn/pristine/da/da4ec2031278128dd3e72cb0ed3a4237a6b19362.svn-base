.. article:: apps_audio_ring_buffer_mcu
    :author: Karel Slanı <slany AT fit.vutbr.cz>
    :updated: 14.10.2009

    Práce s digitálním zvukem kodeku TLV320AIC23B v MCU

===================================
Digitální zvuk TLV320AIC23B v MCU
===================================
.. contents:: Obsah
   :depth: 1

Popis aplikace
===============

V FPGA je z BRAM vytvoøen kruhovı buffer o kapacitì 512 32-bitovıch poloek. Kadá poloka se skládá z dvou 16-bitovıch vzorkù, pro levı a pravı kanál jeden.
Pouitá BRAM má dva porty, které zpøístupòují její obsah. Port A slouí k zápisu/ètení dat do/z kodeku, k portu B je pøipojen øadiè SPI pøipojenı k MCU.
Tento SPI øadiè slouí èistì k pøenosu zvukovıch dat mezi FPGA a MCU a tvoøí tak samostatnı pøenosovı kanál s pøenosovou rychlostí 14MHz v kadém smìru.
MCU pak realizuje ètení zvukovıch dat a jejich zpìtnı zápis. Celá aplikace se chová jako propojení digitálního vıstupu s digitálním vstupem kodeku pøes MCU.

Obsluná aplikace digitalizuje vstupní signál z linkového vstupu JP6 a pøehrává jej na vıstupu JP7.

.. figure:: wire_mcu.png

   Schéma propojení komponent.

Na obrázku `wire_mcu.png`_ je znázornìno blokové schéma zapojení komponent. Bloèky ``codec reader`` a ``codec writer`` jsou realizovány jako samostatné generické komponenty ``aic23b_dsp_read`` a ``aic23b_dsp_write``, které realizují pøevod signálu pro pøenos do kodeku.
Dohromady s ``BRAM`` tvoøí komponentu ``aic23b_dsp_ringbuffer`` její VHDL popis naleznete v souboru `fpga/ctrls/audio/aic23b_dsp_ringbuffer.vhd <SVN_DIR/trunk/fpga/ctrls/audio/aic23b_dsp_ringbuffer.vhd>`_.
Blok ``fast SPI`` realizuje øadiè druhého SPI pro pøenos zvuku. VHDL popis komponenty naleznete v souboru `fpga/spi_controller.vhd <SVN_APP_DIR/fpga/spi_controller.vhd>`_.
Bloky ``BRAM CU`` jsou realizovány jako procesy. Slouí k vıpoètu adresy pro zápis a ètení z BRAM. Jsou øízeny komponentami, které realizují komunikaèní rozhraní pro kodek a MCU.


Ovládání aplikace
------------------

Kodek TLV320AIC23B se dá rozdìlit na dvì èásti - analogovou (zesilovaèe a mixer) a digitální (A/D a D/A sigma delta pøevodník, digitální sériové rozhraní). Analogová èást umoòuje obejít (bypass) digitální èást.
Aktivace digitálního rozhraní se v aplikaci provede pøíkazem ``DSP_ON``, pøíkazem ``BYPASS_DSP`` deaktivujete digitální rozhraní a zapne se bypass. Pokud neslyšíte zvuk ani po zadání pøíkazu ``BYPASS_DSP`` ujistìte se, zda máte správnì pøipojen linkovı vstup.

Pøíkazem ``DSP_ON`` se skrze konfiguraèní SPI rozhraní kodeku nastaví konfiguraèní registry, èím se aktivují A/D D/A pøevodníky, nastaví komunikaèní protokol na DSP reim, vzorkovací frekvence a bitová šíøka vzorkù.

Pøíkaz ``BYPASS_DSP`` deaktivuje digitální pøevodníky a aktivuje analogovı bypass, kterım se propojí analogovı vstup s analogovım vıstupem.

V obou pøedchozích pøípadech, je aktivní vıstup JP7.

Pøíkazem ``MUTE`` se v obou reimech sníí hlasitost vıstupu na minimum. Pøíkaz ``RESTORE`` obnoví pùvodní hlasitost vıstupu.


Rozhraní SPI pro pøenos zvuku
=====================================

MCU je v této aplikaci pøipojeno k FPGA pomocí dvou SPI rozhraní. První SPI je klasické, urèené pro komunikaci s SPI øadièem. Je definováno v knihovnì libfitkit.
Druhé SPI rozhraní slouí pouze k pøenosu digitálního zvuku. Funkce pro MCU naleznete v souborech `mcu/fspi.h <SVN_APP_DIR/mcu/fspi.h>`_ a `mcu/fspi.c <SVN_APP_DIR/mcu/fspi.c>`_.
Toto rozhraní pracuje na 14MHz.

Zprovoznìní aplikace
========================
1. Pøed spuštìním kodeku se ujistìte, e propojky J3, J4 jsou aktivní a J2 je neaktivní.

2. Do konektoru JP7 zapojte sluchátka a do JP6 zdroj signálu (napø. MP3 pøehrávaè).

3. Pøelote aplikaci.

4. Naprogramujte MCU a FPGA a spuste terminálovı program.

   Nyní byste mìli ve sluchátkách slyšet zvuk (vzorkovanı na 8kHz) pøicházející do linkového vstupu JP6.
