.. article:: apps_demo_led
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20090326

    Jednoduch� aplikace demonstruj�c� ��zen� LED diody pomoc� FPGA a pomoc� mikrokontroleru.

========================
Blik�n� diodami LED
========================

Popis aplikace
=================

Tato jednoduch� aplikace demonstruje ��zen� LED diody pomoc� ��ta�e implementovan�ho v FPGA a pomoc� mikrokontroleru. Jedn� se o nejjednodu��� aplikaci, kdy zdrojov� k�dy obsahuj� pouze nezbytn� minimum. 

Aplikace se skl�d� ze dvou ��st�:

* **Blik�n� LED ��zen� z FPGA**

  VHDL k�d obsahuje pouze 23-bitov� ��ta�, co� je komponenta, kter� s ka�dou n�b�nou hranou hodinov�ho sign�lu inkrementuje sv�j obsah. Posledn� bit ��ta�e je p�ipojen na LED diodu D4, kter� tak za�ne ihned po naprogramov�n� FPGA obvodu blikat s periodou cca p�l sekundy. ��ta� je synchronizov�n hodinov�m sign�lem SMCLK s kmito�et 7.3728MHz, kter� generuje mikrokontroler (detaily viz schema kitu).

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

  Zdrojov� k�dy je mo�n� nal�zt v souboru `fpga/top.vhd <SVN_APP_DIR/fpga/top.vhd>`_. 

* **Blik�n� LED ��zen� z MCU**

  K�d v mikrokontroleru, kter� taji��uje periodick� blik�n� LED diody D6 se skl�d� z nekone�n� smy�ky obsahuj�c� funkci ``delay_ms(1)``, kter� realizuje pomoc� voln� b��c�ho ��ta�e zpo�d�n� 1ms. Po 500ti iterac�ch se invertuje bit p�ipojen� na LED diodu. Po naprogramov�n� MCU za�n� periodicky blikat �erven� dioda D6 (pokud je aktivn� termin�l, p��padn� z�stanou rozpojeny propojky J8 a J9).

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

  Zdrojov� k�dy je mo�n� nal�zt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

  .. note:: Funkce terminal_idle zaji��uje obsluhu termin�lu, tzn. p�ij�m� a vy�izuje zaslan� p��kazy. V t�to jednoduch� aplikaci m��e b�t bez probl�m� vypu�t�na.

Zprovozn�n� aplikace
========================
1. p�elo�te aplikaci
2. naprogramujte MCU a FPGA a spus�te termin�lov� program.

   Nyn� by m�la blikat dioda D6 a D4.

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.