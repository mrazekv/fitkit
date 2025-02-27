.. article:: apps_demo_led
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20090326

    Jednoduchá aplikace demonstrující øízení LED diody pomocí FPGA a pomocí mikrokontroleru.

========================
Blikání diodami LED
========================

Popis aplikace
=================

Tato jednoduchá aplikace demonstruje øízení LED diody pomocí èítaèe implementovaného v FPGA a pomocí mikrokontroleru. Jedná se o nejjednodušší aplikaci, kdy zdrojové kódy obsahují pouze nezbytné minimum. 

Aplikace se skládá ze dvou èástí:

* **Blikání LED øízené z FPGA**

  VHDL kód obsahuje pouze 23-bitovı èítaè, co je komponenta, která s kadou nábìnou hranou hodinového signálu inkrementuje svùj obsah. Poslední bit èítaèe je pøipojen na LED diodu D4, která tak zaène ihned po naprogramování FPGA obvodu blikat s periodou cca pùl sekundy. Èítaè je synchronizován hodinovım signálem SMCLK s kmitoèet 7.3728MHz, kterı generuje mikrokontroler (detaily viz schema kitu).

  ::

     LEDF <= cnt(22);
  
     counter: process(RESET, SMCLK)
     begin
         if (RESET = '1') then
             cnt <= (others => '0');
         elsif SMCLK'event and SMCLK = '1' then
             cnt <= cnt + 1;
         end If;
     end process;

  Zdrojové kódy je moné nalézt v souboru `fpga/top.vhd <SVN_APP_DIR/fpga/top.vhd>`_. 

* **Blikání LED øízené z MCU**

  Kód v mikrokontroleru, kterı tajišuje periodické blikání LED diody D6 se skládá z nekoneèné smyèky obsahující funkci ``delay_ms(1)``, která realizuje pomocí volnì bìícího èítaèe zpodìní 1ms. Po 500ti iteracích se invertuje bit pøipojenı na LED diodu. Po naprogramování MCU zaènì periodicky blikat èervená dioda D6 (pokud je aktivní terminál, pøípadnì zùstanou rozpojeny propojky J8 a J9).

  ::

    while (1) 
    {
      delay_ms(1);  //zpozdeni 1ms
      counter++;
      if (counter == 500) 
      {
         flip_led_d6(); //invertovat LED
         counter = 0;
      }
      terminal_idle();
    }

  Zdrojové kódy je moné nalézt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

  .. note:: Funkce terminal_idle zajišuje obsluhu terminálu, tzn. pøijímá a vyøizuje zaslané pøíkazy. V této jednoduché aplikaci mùe bıt bez problémù vypuštìna.

Zprovoznìní aplikace
========================
1. pøelote aplikaci
2. naprogramujte MCU a FPGA a spuste terminálovı program.

   Nyní by mìla blikat dioda D6 a D4.

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.