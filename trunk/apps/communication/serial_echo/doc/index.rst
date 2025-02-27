.. article:: apps_communication_serial_echo
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20090404

    Komunikace mezi øadièem sériové linky v FPGA a PC (komunikaèním kanálem A obvodu FTDI)

===============================
Seriová linka v FPGA (echo)
===============================

Popis aplikace
=================

Cílem aplikace je ukázat, jak je moné uvnitø FPGA vytvoøit øadiè seriové linky a jak lze pomocí øadièe pøenášet data z/do PC.
Øadiè je pøipojen k `USB pøevodníku FTDI <DOChw_ftdi>`_, kanálu A.

.. note:: Jeliko data v této jednoduché aplikaci putují (kvùli zjednodušení kódu) zámìrnì znak po znaku z FPGA do MCU a zpìt, je nutné mít na pamìti s tím související problémy. Vzhledem k latenci mezi vyvoláním ádosti o pøerušení, vlastní obsluhou pøerušení a vyètení pøijatého bytu pøes SPI mùe docházet ke ztrátì pøijatıch dat v pøípadì, e bude odvysíláno více bytù souèasnì. Obyèejnì není potøeba data pøeposílat pøes MCU a pokud ano, pak je nutné vytvoøit uvnitø FPGA FIFO pamì, která bude shromaïovat pøijaté byty.

Zdrojové kódy aplikace lze nalézt v `SVN <SVN_APP_DIR>`_.


Zprovoznìní aplikace
=====================

1. pøelote aplikaci 

   ::

     fcmake && make 


2. pro správnou èinnost je nutné povolit pøerušení z FPGA do MCU

   *  FITkit verze 1.x:
       
      propojit pin JP10(5) vedoucí z FPGA s pinem JP9(26) vedoucím do MCU

   *  FITkitu verze 2.x:
     
      propojit propojku J5, která je urèena k povolení pøerušení

3. naprogramujte MCU a FPGA a spuste terminálovı program (napø. QDevKit), pøípadnì rozpojte propojky J8,J9

    ::

      make load
      make term

4. pøipojte se pomocí vhodného terminálového programu na kanál A USB pøevodníku (buï pøímo nebo pøes virtuální COM port, jeho èíslo naleznete ve správci zaøízení - viz dokument `DOCinstall_ft2232c`_).
   Parametry nastavte následovnì: komunikaèní rychlost 57600 Bd, 8 datovıch bitù, lichá parita, 1 stop bit.

   .. figure:: serialsettings.png

      Hyperterminál

   Pøi kadém stisku klávesy v terminálu pøipojeném ke kanálu A se v tomté terminálu zobrazí odpovídající znak (echo) a dioda D4 zhasne/se rosvítí. Jste-li pøipojeni k mikrokontroleru, je kadı pøijatı znak zobrazen také v pøíslušném terminálovém oknì QDevKitu.
