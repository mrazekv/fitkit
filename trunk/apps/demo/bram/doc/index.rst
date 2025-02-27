.. article:: apps_demo_bram
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20090505

    Aplikace demonstruje zpøístupnìní blokové pamìti BRAM, která je integrovaná uvnitø FPGA, pøes rozhraní SPI. 
    Pamì je pøipojena k mikrokontroléru a pomocí pøíkazù pro zápis a ètení do/z SPI je moné modifikovat a èíst její obsah.

==========================================
Pøístup k pamìti BRAM z mikrokontroleru
==========================================

Popis aplikace
==================

Tato aplikace demonstruje pøipojení blokové pamìti ke komunikaèní sbìrnici SPI, pomocí které je moné z mikrokontroleru èíst a modifikovat obsah pamìti.
Pamì samotná mùe slouit napø. pro uchování informací o obrazu (viz grafické aplikace), fontu èi jinıch dat.

**Bloková pamì RAM (BRAM)**

FPGA obvod pouitı ve FITkitu obsahuje ètyøi blokové pamìti BRAM o celkové kapacitì 4 x 16384 bitù. 
Kadá pamìt mùe bıt pouita v závislosti na poadavcíc aplikace jako dvouportová nebo jednoportová. 
V prvním pøípadì se vyuívá primitiva ``RAMB16_Sm_Sn``, v druhém pøípadì primitiva ``RAMB16_Sn``, kde konstanty *m* a *n* specifikují datovou šíøku prvního a druhého portu a mohou nabıvat následujících hodnot: 1, 2, 4, 9, 18 nebo 36 bitù. 
V pøípadì datové šíøky 9, 18 a 36 bitù má uivatel k dispozici 8, 16 a 32 datovıch bitù a navíc 1, 2 pøípadnì 4 paritní bity.
Paritní bity mohou bıt vyuity také k úschovì dat, nebo vıpoèet parity není provádìn automaticky.

Uivatel má tedy monost nainstancovat pamì s datovou šíøkou, která nejlépe vyhovuje dané aplikaci a tudí mùe uspoøit bity, které by byly jinak nevyuity.
Pamì BRAM je synchronní a v pøípadì dvouportové varianty jsou k dispozici dva nezávislé hodinové vstupy. Nejèastìji se pouívá pro implementaci vyrovnávacího bufferu FIFO, pøípadnì datového úloištì o malé kapacitì.


Návrh systému
================

VHDL kód pro FPGA
--------------------

Pro komunikaci mikrokontroleru s pamìtí BRAM je pouit adresovı dekodér SPI (viz dokument `DOCfpga_interconect`_), kterı je adresován pomocí 16-bitù široké adresy. Data jsou pøenášena do a z pamìti po 16ti bitech. Zpùsob zapojení jednotlivıch komponent uvnitø FPGA je zobrazen na následujícím obrázku.

.. figure:: bram.png
   
   Blokové schéma aplikace

Zdrojové kódy je moné nalézt v souboru `fpga/top.vhd <SVN_APP_DIR/fpga/top_level.vhd>`_. 

Firmware pro mikrokontroler
-------------------------------
Program pro mikrokontroler pouívá k pøenosu 16bitovıch slov z/do pamìti funkci ``FPGA_SPI_RW_AN_DN`` a je navren tak, e je schopen obsah pamìti zinicializovat, pøeèíst a vyplnit postupnì se zvyšující hodnotou.

Ukázka kódu realizujícího zápis 16-bitového slova na 16-bitovou adresu

  ::

    unsigned short val = 0x1234;

    //zapis 16 bitove hodnoty val na adresu BRAM_BASE + offset
    FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BRAM_BASE + offset, (unsigned char *)&val, 2, 2);
  
Ukázka ètení 16-bitového slova z 16-bitové adresy s vyuitím stejné funkce

  ::

    unsigned short val;

    //cteni 16 bitove hodnoty z adresy BRAM_BASE + offset
    FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, BRAM_BASE + offset, (unsigned char *)&val, 2, 2);

Zdrojové kódy je moné nalézt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovoznìní aplikace
========================
1. pøelote aplikaci
2. naprogramujte MCU a FPGA a spuste terminálovı program
3. pomocí pøíkazu ``help`` je moné vypsat seznam pøíkazù souvisejících s touto aplikací

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.
