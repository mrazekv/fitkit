.. article:: apps_communication_1-wire_mcu
    :author: Karel Slanı <slany AT fit.vutbr.cz>
    :updated: 20100209

    Implementace sbìrnice 1-wire pro MCU a komunikace s èidlem DS18B20.

================================
Sbìrnice 1-wire a èidlo DS18B20
================================

Èidlo DS18B20
=================
Souèástka DS18B20[DS18B20]_ je bìnì dostupnı digitální teplomìr, kterı je schopnı mìøit v rozsahu -55°C a +125°C. Vırobce garantuje pøesnost 0.5°C v teplotním rozsahu -10°C do +85°C. Pøesnost mìøení je volitelná v rozmezí 9 - 12 bitù (tzn. rozlišení 0.5 - 0.0625°C).

.. [DS18B20] DS18B20 Programmable Resolution 1-Wire Digital Thermometer http://pdfserv.maxim-ic.com/en/ds/DS18B20.pdf

Senzor se pøipojuje pomocí jedno-vodièového seriového rozhraní oznaèované jako 1-wire[1wire]_.

.. [1wire] 1-Wire wikipedia entry WWW: http://en.wikipedia.org/wiki/1-Wire

Pro komunikaci se pouívá jeden datovı a dva napajecí vodièe (GND, VDD, DQ). Napájecí napìtí VDD se mùe pohybovat v rozmezí 3.3V a 5.5V. Mimo to lze senzor provozovat v tzv. parazitním reimu vyuívajícím pouze dvou vodièù (GND, DQ). V tomto typu zapojení je pøipojené zaøízení napájeno interním kondenzátorem, kterı je pøipojen k datovému vodièi. K datovému vodièi je nutné pøipojit slabı pull-up rezistor, protoe všechna zaøízení jsou pøipojena ke sbìrnici skrze otevøenı kolektor.

.. figure:: ds18b20multi2.png

   Obvod DS18B20 a zpùsob zapojení více senzorù na sbìrnici

Kromì mìøení teploty obvod CD18B20 disponuje funkcí digitálního termostatu, jeho dolní a horní hranici lze programovì nastavit a uloit do EEPROM pamìti na èipu. Pøi ètení namìøené hodnoty jsou pak k dispozici bity informující o pøekroèení naprogramovanıch rozsahù.

Kadı obvod má ji z vıroby pøidìleno své vlastní unikátní 64-bitové seriové èíslo, které slouí k adresaci konkrétního senzoru. Seriové èíslo spoleènì s unikátním protokolem identifikace pøipojenıch zaøízení[1-wsearch]_ ke sbìrnici umoòuje provozovat nìkolik senzorù na jednom datovém vodièi.

.. [1-wsearch] 1-Wire Search Algorithm http://pdfserv.maxim-ic.com/en/an/AN187.pdf

Popis aplikace
=================
Aplikace umoòuje snímat teplotu z libovolného poètu senzorù DS18B20 pøipojenıch pøímo k jednomu z pinù MCU. Maximální poèet senzorù je omezen velikostí dostupné pamìti. Ke kadému zaøízení je toti nutné uchovat jeho 64-bitové identifikaèní èíslo. To pak slouí k adresaci zaøízení na sbìrnici.
Komunikaci s èidly a implementaci 1-wire protokolu zajišuje program[1-wsw]_ bìící v MCU. 
Mikrokontroler obsahuje rutiny, které jsou schopny dentifikovat zarizeni na bìrnici 1-wire, znicializovat teplotní èidla a zmìøit teplotu.

.. [1-wsw] 1-Wire® Communication Through Software http://pdfserv.maxim-ic.com/en/an/AN126.pdf

Zdrojovı kód protokolu 1-wire lze nalézt v souborech `one_wire.h <SVN_APP_DIR/mcu/one_wire.h>`_ a `one_wire.c <SVN_APP_DIR/mcu/one_wire.c>`_.

Funkce ``ow_find_devices()`` identifikuje zaøízení na sbìrnici a uloí jejich identifikaèní èísla do interních datovıch struktur.

Vlastní mìøení realizují rutiny ``ds_measure_all()`` a ``ds_get_temperature_selected(idx)`` definované v souboru `file:main.c`. Sbìrnice 1-wire umoòuje souèasnì rozeslat všem zaøízením jeden pøíkaz. Toho lze vyuít v pøípadì, kdy je ke sbìrnici pøipojeno pouze jediné zaøízení (protoe se vıznamnì zjednoduší komunikaèní procedury - není nutné adresovat zaøízení) nebo v pøípadech, e na sbìrnici je více stejnıch zaøízení vykonávajících stejné operace. Funkce ``ds_measure_all()`` rozešle všem zaøízením pøíkaz k mìøení teploty. Funkce ``ds_get_temperature_selected(idx)`` se pak dotazuje jednotlivıch zaøízení na namìøenou hodnotu. Pro identifikaci zaøízení slouí parametr ``idx``, kterı je èíslo z intevalu <0, (devs-1)>, kde devs je poèet pøipojenıch zaøízení ke sbìrnici.

Zdrojové kódy aplikace lze nalézt v `SVN <SVN_APP_DIR>`_.

.. figure:: ds18b20fitkit_conn.png

   Zapojení dvou DS18B20 k FITkitu.

Zprovoznìní aplikace
=====================

   1. pøelote aplikaci

   2. pøipojte teplotní senzor na pin 6 konektoru JP9 dle nákresu na obrázku. Datovı vodiè je nutné opatøit pull-up resistorem o odporu nìkolik kilo ohmù.

   3. naprogramujte MCU a FPGA a spuste terminálovı program nebo vytáhnìte propojky J8/J9
      
      Pokud je senzor pøipojen, mìl by se po resetu MCU na displeji zobrazit poèet nalezenıch zaøízení. Po chvíli by se mìla zaèít zobrazovat namìøená teplota ze všech senzorù, která se aktualizuje kadou sekundu.
