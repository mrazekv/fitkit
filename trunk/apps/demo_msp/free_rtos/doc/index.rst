.. article:: apps_demo_msp_freertos_lamp
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>; Zdenek Vasicek <vasicek AT fit.vutbr.cz>
    :updated: 20100321

    Ovl�d�n� sv�tidla pomoc� RT aplikace 

===============================================
FreeRTOS (ovl�d�n� sv�tidla pomoc� RT aplikace)
===============================================

  .. note:: V�ce o FreeRTOS je mo�no nal�zt na www.freertos.org

.. figure:: free_rtos.png
   :align: center

.. contents:: Obsah

Popis aplikace
==================

S vyu�it�m prost�edk� RT j�dra FreeRTOS  
je pro MSP430 na FITkitu v jazyce C implementov�na jednoduch� RT aplikace (``Sv�tidlo`` - CZ, ``Lamp`` - EN)
tvo�en� 

1. ``tla��tkem`` (pro jednoduchost je vyu�ito tla��tko ``A`` z maticov� kl�vesnice FITkitu), 

2. ``��dic� ��st�`` (implementovanou v MSP430 pomoc� prost�edk� FreeRTOS) reaguj�c� na stisk tla��tka a 

3. ``��zenou ��st�`` reprezentovanou pro jednoduchost LED diodou ``D5`` (zelen� LED um�st�n� v lev�m doln�m rohu FITkitu).

Aplikaci je mo�no slovn� ``specifikovat`` n�sledovn�. Sv�tidlo se m��e nach�zet v jednom ze 3 stav�:

1. ``OFF`` (zhasnuto), 

2. ``LOW`` (sv�t� slab�), 

3. ``BRIGHT`` (sv�t� siln�). 

Po��te�n� stav sv�tidla je stav ``OFF``.
Po (prvn�m) stisku tla��tka za�ne sv�tidlo sv�tit slab� (tj. p�ejde do stavu ``LOW``).  
Bude-li tla��tko stisknuto do 500 ms po p�echodu do stavu ``LOW``, pak za�ne sv�tidlo sv�tit siln� (tj. p�ejde do stavu ``BRIGHT``). 
Sv�tidlo zhasne (tj. p�ejde do stavu ``OFF``) bude-li tla��tko stisknuto a) za v�ce ne� 500 ms od p�echodu do stavu ``LOW`` nebo b) p�i siln�m svitu sv�tidla (tj. ve stavu ``BRIGHT``). 

.. figure:: lamp_small.png
   :align: center

   Model chov�n� slovn� specifikovan� aplikace pomoc� dvojice komunikuj�c�ch ``�asovan�ch automat�`` (``Timed Automaton``, ``TA``).
   Pozn.: ``y`` je prom�nn� typu ``clock`` ur�en� k odm��ov�n� �asu, ``press`` je kan�l, p�es kter� automaty komunikuj� 
   (``press?`` p�edstavuje �ek�n� na p��chod zpr�vy po kan�lu ``press`` zat�mco 
   ``press!`` p�edstavuje vysl�n� zpr�vy po kan�lu ``press``)

.   
  .. note:: Pr�ci s TA lze vyzkou�et nap�. pomoc� voln� dostupn�ho n�stroje ``UPPAAL`` - viz. ``www.uppaal.com``

Realizace aplikace pomoc� prost�edk� RT j�dra FreeRTOS
======================================================
  .. note:: ``RT j�dro`` se �asto ozna�uje pojmem ``real-time opera�n� syst�m``, pop�. ``opera�n� syst�m pro pr�ci v re�ln�m �ase`` apod., obvykl� zkratka je ``RTOS``

Aby bylo mo�n� realizovat danou aplikaci pomoc� prost�edk� RT j�dra,
je nutn� zamyslet se nad rozvr�en�m d�l��ch ��st� aplikace do mno�iny tzv. ``RT �loh``.
Konkr�tn� realizace, tedy pomoc� FreeRTOS, m��e m�t tuto podobu:

1. stav tla��tka 'A' je testov�n �lohou ``keyboardTask``,
kter� je vol�na s periodou ``1 ms``. 
Stisk tla��tka je signalizov�n nastaven�m glob�ln� prom�nn� 
``press`` na hodnotu 1. Toto �e�en� m� sv� nev�hody 
(nap�. aktivn� �ek�n� �lohy na p��kazu ``while(!pressed);``), ale je jednoduch�
a pro na�e ��ely posta�uje. Kostra �lohy:

::

  static void keyboardTask(void *param) 
  {
    ...
    keyboard_init();
  
    for (;;) 
    {
      ...
      ch = key_decode(read_word_keyboard_4x4());
        ...
        if (ch != 0) {
           switch (ch) {
             case 'A':
               pressed = 1;
            ...
           }
        }
     }
     /* sleep task for 1 ms (= btn-press sampling period) */ 
     vTaskDelay( 1 / portTICK_RATE_MS); 
    }
  }

2. reakci na stisk tla��tka 'A' v z�vislosti na stavu aplikace zaji��uje �loha 
``lampTask``. V jednotliv�ch stavech �lohy se m�n� hodnota prom�nn�
``lightIntensity`` ur�uj�c� intenzitu svitu sv�tidla.
Kostra �lohy je n�e:

::

  static void lampTask(void *param) 
  {
    pressed = 0;
  
    for(;;)
    {
      /*--- OFF ---*/
      lightIntensity = lightOFF;
      while(!pressed); 
      pressed = 0;
      
      /*--- LOW ---*/
      lightIntensity = lightLOW;
      vTaskDelay( 500 / portTICK_RATE_MS); /* wait for 500 ms */ 
  
      if(pressed)
      { 
        /*--- BRIGHT ---*/
        pressed = 0;
        lightIntensity = lightBRIGHT;
      }

      while(!pressed); 
      pressed = 0;
    }
  }

3. svit sv�tidla je �e�en v obsluze p�eru�en� od �asova�e B (TIMERB).
Podle aktu�ln� hodnoty prom�nn� ``lightIntensity`` zajist�
�asova� odpov�daj�c� svit LED diody ``D5``:

::

  interrupt (TIMERB0_VECTOR) Timer_B (void)
  {
    cnt++;
  
    if((cnt>0) && (cnt<=lightIntensity)) { set_led_d5(1); } /* D5 on-time */
    else { set_led_d5(0);}  /* D5 off-time */
  
    if(cnt>=(MAX_LIGHT_LEVELS)) cnt=0;
  
    TBCCR0 = 0x1000;          /* interrupt each x ticks of clk source */
  }


P��kazov� ��dka aplikace
=========================

O p��padnou obsluhu p��kazov� ��dky se star� �loha ``terminalTask``,
kter� je vol�na s periodou 1 s:

::

  static void terminalTask(void *param) 
  {
    for (;;) {
      terminal_idle();
      vTaskDelay( 1000 / portTICK_RATE_MS); /* delay for 1000 ms */ 
    }
  }

Inicializace a spu�t�n� aplikace
================================

Viz ``main.c`` (kostra):

::

  int main( void ) 
  {
    initialize_hardware();
    TBCCTL0 = CCIE;           /* TIMER B interrupts enable */
  
    /*--- install FreeRTOS tasks ---*/
    term_send_str_crlf("init FreeRTOS tasks...");
    xTaskCreate(terminalTask /* code */, "TERM" /* name */, 100 /* stack size */, NULL /* params */, 1 /* prio */, NULL /* handle */);
    xTaskCreate(keyboardTask, "KBD", 32, NULL, 1, NULL);
    xTaskCreate(lampTask, "LAMP", 32, NULL, 1, NULL);
  
    /*--- start FreeRTOS kernel ---*/
    term_send_str_crlf("starting FreeRTOS scheduler...\n");
    return 0;
  }

Zdrojov� k�dy
=============

Kompletn� zdrojov� k�dy je mo�n� nal�zt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovozn�n� aplikace
========================
1. p�elo�te aplikaci

2. naprogramujte MCU a FPGA a spus�te termin�lov� program.

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.

