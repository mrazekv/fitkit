.. article:: apps_communication_ds18b20
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20090404

    Komunikace s teplotním èidlem DS18B20 pøipojenım ke sbìrnici one-wire.

===============================
Mìøení teploty pomocí DS18B20
===============================

Senzor DS18B20
=================
`DS18B20 <http://pdfserv.maxim-ic.com/en/ds/DS18B20.pdf>`_ je bìnì dostupnı digitální teplomìr, kterı je schopen mìøit v rozsahu -55°C a +125°C s pøesností ±0.5°C garantovanou v teplotním rozsahu -10°C to +85°C. 
Data jsou ze senzoru získávána pomocí jedno-vodièového seriového rozhraní (`1-wire interface <http://en.wikipedia.org/wiki/1-Wire>`_) s uivatelsky programovatelnou pøesností 9 - 12 bitù (tzn. rozlišení rozlišení 0.5 - 0.0625°C). 
Pro komunikaci se pouívá jeden datovı a dva napajecí vodièe (GND, VDD, DQ). Napájecí napìtí VDD se mùe pohyboat v rozmezí 3.3V a 5.5V. Mimo to lze senzor provozovat v tzv. parazitním reimu vyuívajícím pouze dvou vodièù.

.. figure:: ds18b20multi2.png

   Obvod DS18B20 a zpùsob zapojení více senzorù na sbìrnici

Kromì mìøení teploty obvod disponuje funkcí digitálního termostatu, jeho dolní a horní hranici lze programovì nastavit a uloit do EEPROM pamìti na èipu. Pøi ètení namìøené hodnoty jsou pak k dispozici bity informující o pøekroèení naprogramovanıch rozsahù.

Kadı obvod má ji z vıroby pøidìleno své vlastní unikátní 64-bitové seriové èíslo, které slouí k adresaci konkrétního senzoru. Seriové èíslo spoleènì s unikátním protokolem identifikace pøipojenıch zaøízení ke sbìrnici umoòuje provozovat nìkolik senzorù na jednom datovém vodièi. Mimo to nabízí obvod monost vyuít napìovı signál pro pøenos dat, èím se celkovı poèet vodièù nutnı k pøipojení senzorù redukuje na dva (GND, VDD/DQ).

Popis aplikace
=================
Aplikace umoòuje snímat teplotu z jednoho senzoru DS18B20 pøipojeného k jednomu z pinù FPGA obvodu.
Komunikaci s èidlem a realizaci 1-wire protokolu zajišuje øadiè uvnitø FPGA. 
Mikrokontroler obsahuje rutiny, které jsou schopny znicializovat teplotní èidlo a zmìøit teplotu. Rutiny vyuívají pøíkazu ètení a zápisu bytu poskytovaného FPGA øadièem.

Øadiè v FPGA se skládá z koneèného automatu, kterı zajišuje a) ètení 8-bitové informace, b) zápis 8-bitové informace, c) inicializaci teplotního sensoru a detekci jeho pøipojení. 
Zdrojovı kód øadièe lze nalézt v souboru `one_wire.vhd <SVN_APP_DIR/fpga/one_wire.vhd>`_.

Vlastní mìøení realizuje rutina ``measuretemp()`` uvedená v souboru `file:main.c`.
Tato funkce vyuívá následující jednoduché rutiny komunikující s øadièem v FPGA:
* ``onewire_reset()`` - naøídí øadièi v FPGA zinicializovat zaøízení (je-li nìjaké pøipojeno)
* ``onewire_write()`` - øadiè v FPGA pošle pøes 1-wire 8-bitové slovo
* ``onewire_read()`` - øadiè v FPGA pøeète z 1-wire 8-bitové slovo a vrátí je mikrokontoleru (v rutinì se aktivnì èeká na dokonèení operace)

Pomocí tìchto funkcí jsou zasílány pøíkazy a èteny data ze senzoru.
Nejdøíve je zahájen pøevod teploty (pøíkaz 0xCC 0x44), poté se aktivnì èeká, a je pøevod dokonèen a následnì je pøeètena namìøená hodnota (pøíkaz 0xCC 0xBE). Bliší informace naleznete v pøíslušném datasheetu.

Zdrojové kódy aplikace lze nalézt v `SVN <SVN_APP_DIR>`_.


Zprovoznìní aplikace
=====================

   1. pøelote aplikaci

   2. pøipojte teplotní senzor na pøedposlední pin horní øady konektoru JP10 dle nákresu na obrázku `ds18b20fitkit.png`_. Datovı vodiè je nutné opatøit pull-up resistorem o odporu nìkolik kilo ohmù.

      .. figure:: ds18b20fitkit.png

         Pøipojení teplotního èidla k FITkitu

   3. naprogramujte MCU a FPGA a spuste terminálovı program nebo vytáhnìte propojky J8/J9
      
      Pokud je senzor pøipojen, mìla by se na displeji zobrazit namìøená teplota, která se aktualizuje kadou sekundu.
