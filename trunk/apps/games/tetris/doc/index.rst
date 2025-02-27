.. article:: apps_games_tetris
    :author: Izidor Matušov <xmatus19 AT stud.fit.vutbr.cz>
    :updated: 20110131

    Hra Tetris bìící na externím monitoru pøipojeném k rozhraní VGA

========================
Hra Tetris
========================

Popis aplikace 
=================

.. image:: tetris.png
   :align: right

Tato aplikace umoòuje si zahrát oblíbenou hru Tetris na vašem FITkitu. Herní plocha se zobrazuje na externím monitoru, kterı je pøipojen pøes rozhraní VGA.

Aplikaci je moné spustit pøíkazem START pøes terminál. 
Pokud vás nebaví zaèínat s prázdnım polem, mùete pomocí pøíkazu LEVEL vygenerovat náhodnì øádky, které je zapotøebí v prùbìhu hry odstranit.
Hru lze té spustit stiskem libovolné klávesy na FITkitu. Je-li stisknuta klávesa 1-9, dojde pøi spuštìní k nastavení pøíslušného levelu (poètu èásteènì sloenıch øádkù na poèátku hry).
Tetris má bìné ovládání - doleva, doprava, otoèit a posunout kostièku níe. Hrací kostièka se po uplynutí èasového limitu posunuje níe. Rychlost, jakou se kostièky posouvají, je moné mìnit pomocí pøíkazu SPEED. 
S poètem odebranıch øádkù se rychlost posouvání zvyšuje. Souèasná rychlost je zobrazena vpravo pod nápisem Speed. 
Poèet úspìšnì odstranìnıch øádkù se zobrazuje na LCD displeji FITkitu. 
Jakmile kostièka narazí na jinou nebo na dno hrací plochy, není ji moné s ní hıbat.
Z hrací se odstraní vyplnìné øádky a hráè dostává novou kostièku.
Následující kostièka je zobrazena vlevo pod nápisem Next.

Hra konèí v momentì, kdy se další kostièka ji nevejde na hrací plochu.

Zdrojové kódy je moné nalézt v  `SVN repozitáøi <SVN_APP_DIR>`_.

.. figure:: go.png

   Pøíjemnou zábavu pøi hraní


Ovládání hry
=================

Konfigurace a spuštìní hry pomocí pøíkazù zadanıch v terminálu:

 * **SPEED N** - nastaví rychlost padání kostièek (0-9)  
 * **LEVEL N** - nastaví poèet pøedvyplnìnıch øádkù u nové hry (0 - 15)        
 * **START** - spustí novou hru                                              
 * **STOP** - ukonèí aktuální hru                                           
 * **RESTART** - ukonèí aktuální hru a spustí novou                           
 * **HELP** - vypíše nápovìdu                                              

Spuštìní a ovládání hry pomocí klávesnice FITkitu:

 * **1-9** -  start, level 1-9 
 * **4** -  doleva           
 * **6** -  doprava          
 * **8** -  dolù             
 * **2 alebo 5** -  otoèení kostky   
 * **ostatní** -  start, level 0   


Zprovoznìní aplikace
========================
 1. Aktivujte propojku J6 pro povolení periferií PC
  
 2. Pomocí aplikace QDevKit pøelote aplikaci, naprogramujte FITkit a spuste terminál

