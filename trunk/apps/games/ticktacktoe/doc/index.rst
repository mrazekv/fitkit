.. article:: apps_games_tictactoe
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20090326
    
    Hra piškvorky urèená pro jeden/dva kity. Hrací plocha je zobrazována na monitoru, který je pøipojen k rozhraní VGA.

========================
Hra piškvorky
========================

Popis aplikace
=================

.. image:: ticktacktoe.png
   :align: right

Cílem aplikace je ukázat, jak je možné vytvoøit jednoduchou hru pro dva hráèe. Vykreslování èást o kterou se stará FPGA je založena na aplikaci `zobrazující textury <DOCapps_vga_texture>`_. Textury (koleèko, køížek, prázdný a plný ètvereèek) jsou však v tomto pøípadì poèítány v mikrokontroleru a pøeneseny do BRAM pamìtí v FPGA pøed zahájením hry. 

Hru mohou hráèi hrát støídavì na jednom FITkitu a nebo využít propojení dvou kitù, kdy každý hráè mùže hrát na svém kitu. Pøed zahájením hry se provádí detekce kitù a kity se musí dohodnoout, který bude zaèínat jako první.
K tomuto je zapotøebí zdroj pseudo náhodných èísel. V našem pøípadì byl použit teplotní sensor, který je integrován v mikrokontroleru, jehož nepøesnost je v této aplikaci výhodou.

Prùbìh hry je zobrazován na monitoru pøipojeném k portu VGA. 
Na LCD displeji FITkitu je možno najít informace o tom, který hráè je na tahu a pøípadnì který vyhrál.

Zdrojové kódy aplikace je možné nalézt `zde <SVN_APP_DIR>`_.


Použití
=================

Ihned po zapnutí se provádí detekce, zda-li je pøipojen druhý kit (každý kit si vygeneruje náhodné èíslo a snaží se domluvit, kdo bude hru øídit). 
Pokud se podaøí kit detekovat, hraje každý hráè na svém kitu, ke kterému je pøipojen monitor. 
Pokud se nepodaøí detekovat druhý kit, støídají se hráèi na jednom kitu.

K ovládání lze použít klávesnici FITkitu. Pohyb do svislých a vodorovných smìrù umožòují klávesy 2 (nahoru), 4 (doleva), 6 (doprava) a 8 (dolù). Klávesa 5 slouží k položení symbolu - køížek/koleèko.

Po zadání pøíkazu RESTART do terminálu se provede restart hry.


Zprovoznìní aplikace
========================
 1. pøeložte aplikaci 

    ::

      fcmake && make 

 2. Aktivujte propojku J6 pro povolení periferií PC
  
 3. Pro správnou èinnost je nutné povolit pøerušení z FPGA do MCU

    *  FITkit verze 1.x:
       
       propojit pin JP10(5) vedoucí z FPGA s pinem JP9(26) vedoucím do MCU

    *  FITkitu verze 2.x:
     
       propojit propojku J5, která je urèena k povolení pøerušení


 4. Aby fungovala hra na dvou kitech, je nutne spojit následující piny:
    *  FITkit verze 1.x:
       
       pin JP10(47) prvního kitu s pinem JP10(48) na druhém kitu a
       
       pin JP10(48) prvního kitu s pinem JP10(47) na druhém kitu

    *  FITkitu verze 2.x:
      
       pin JP10(49) prvního kitu s pinem JP10(50) na druhém kitu a
      
       pin JP10(50) prvního kitu s pinem JP10(49) na druhém kitu

    jinou možností je použít køížený kabel a konektor RS232, který je na kitu

 5. naprogramovat MCU a FPGA

    ::

      make load

 6. spustit terminálový program, pøípadnì rozpojit propojky J8,J9 a stisknout tlaèítka reset na obou kitech

    ::

      make term
