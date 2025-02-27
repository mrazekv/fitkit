.. article:: apps_demo_picoblaze_led
   :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
   :updated: 11.10.2009

   Jednoduchá aplikace demonstrující øízení LED diody pomocí procesoru PicoBlaze implementovaného uvnitø FPGA.

==================================================
Blikání LED diodou (ralizace zpoïovací smyèky)
==================================================

Popis aplikace
=================
Tato jednoduchá aplikace demonstruje øízení LED diody pomocí 8-bitového procesoru Picoblaze od firmy XILINX, kterı je implementován uvnitø FPGA jako komponenta PicoCPU. 
PicoBlaze je øízen programem, kterı pravidelnì kadou sekundu inkrementuje hodnotu jednoho registru a tuto hodnotu zapisuje na vıstupní port procesoru, kde je pøipojena lutá LED dioda.
Protoe je dioda pøipojena na nejniší bit, bliká v intervalu jedna sekunda.


Návrh aplikace (FPGA)
=======================

V této aplikaci jsou pouity dva adresové dekodéry SPI_ADC (viz `DOCfpga_interconect`_) a komponenta `PicoCPU. <DOCfpga_picocpu>`_ Komponenta PicoCPU je propojena s jedním z adresních dekodérù, díky nìmu je umonìn pøístup k pamìti programu. Druhı adresní dekodér je vyuit pro øízení procesoru - umoòuje jeho spuštìní a zastavení. Z toho dùvodu je za tímto dekodérem pøipojen registr, kterı drí uloenu zapsanou hodnotu.

.. figure:: app_demo_pled.png

   Propojení komponent uvnitø FPGA

Protoe procesor PicoBlaze pouívá z dùvodu efektivního vyuití pamìtí BlockRAM 18-bitové instrukce, je pro jednoduchost pouita v dekodéru 24 bitová datová šíøka slova. Adresace pamìti probíhá prostøednictvím 10-bitového adresního signálu. Nastavení generickıch parametrù pro SPI dekodér slouících pro pøenos instrukcí programu:

  * `generic:ADDR_WIDTH` = 16 (šíøka adresy)
  * `generic:DATA_WIDTH` = 24 (šíøka dat)
  * `generic:ADDR_OUT_WIDTH` = 10 (šíøka vıstupní adresy)
  * `generic:BASE_ADDR` = 16#F800# (bázová adresa SPI dekodéru).

Druhéhı adresní dekodér slouí pouze k pøenosu jednobitové informace, která povoluje/zakazuje chod procesoru. Instrukce pro procesor lze nahrát do pamìti pomocí terminálu. Ukázkovı kód je moné najít v SVN ve sloce `data <SVN_APP_DIR/data>`_. Zdrojovı kód je nutné pøeloit pøekladaèem ``KCPSM3``, kterı je souèástí balíku PicoBlaze, jeho popis mùete nalézt na stránce `DOCfpga_picocpu`_.

Návrh aplikace (Picoblaze - counter.psm)
==========================================
Program pro procesor PicoBlaze je napsán v Assembleru a obsahuje dvì rutiny (``main`` a ``delay``). Hlavní programová smyèka zapisuje na port hodnotu registru ``r7`` a tuto hodnotu inkrementuje. 
Úkolem rutiny ``delay`` je realizovat co nejpøesnìji zpodìní 1 sekundu pomocí smyèek. Zde vycházíme ze znalosti frekvence, na které je PicoBlaze provozován a dále ze znalosti doby trvání jedné instrukce, co je dle datasheetu dva hodinové cykly.

Návrh aplikace (MCU)
=======================
Pøedtím, ne procesor PicoBlaze spustíme, je zapotøebí nahrát do instrukèní pamìti program. Program lze nahrát pomocí terminálu. 

Program pro MCU vychází øeší pøíjem a zpracování souboru `file:.hex`, kterı je vıstupem pøekladaèe assembleru KCPSM3. Tento soubor je mikrokontrolerem pøijat a uloen do pamìti FLASH, aby nebylo nutné vdy po spuštìní FITkitu program nahrávat znovu. K zápisu do FLASH slouí pøíkaz ``FLASH W HEX``. Soubor `file:.hex`  obsahuje vdy 1024 instrukcí. Na jednom øádku je uloena právì jedna instrukce. Kadá instrukce je uloena jako hexadecimální èíslo. 

Po spuštìní/resetu FITkitu je program pro PicoBlaze automaticky odeslán z pamìti FLASH prostøednictvím sbìrnice SPI do FPGA, kde ho pøijímá adresní dekodér a zapisuje do BlockRAM pamìti nacházející se uvnitø PicoCPU. Aktualizaci programu z pamìti FLASH je moné provést pomocí pøíkazu ``CPU UPDATE``. Pro povolení pøípadnì zakázání vykonávání instrukcí procesorem PicoBlaze je moné vyuít pøíkazy ``CPU START`` a ``CPU STOP``.

Zprovoznìní aplikace
========================
   1. Ze stránek XILINX stáhnìte implementaci PicoBlaze v jazyku VHDL. Dle pokynù uvedenıch v `DOCfpga_picocpu`_ zprovoznìte komponentu PicoCPU.

   2. Pøelote aplikaci

   3. Spuste terminálovı program QDevKit a naprogramujte MCU a FPGA

   4. Zahajte komunikaci s FITkitem a pomocí pøíkazu ``FLASH W HEX`` nahrajte program pro procesor PicoBlaze umístìnı v `file:data/counter.hex` do pamìti FLASH umístìné na FITkitu.

      Kód pro PicoBlaze lze v pøípadì potøeby ze zdrojového souboru pøeloit pomocí pøíkazu ``kcpsm3.exe counter.psm``.

   5. Pomocí pøíkazu ``CPU UPDATE`` inicializujte pamì programu uvnitø FPGA a povolte èinnost procesoru.

   6. Procesor zaène vykonávat instrukce èím bude blikat dioda D4
