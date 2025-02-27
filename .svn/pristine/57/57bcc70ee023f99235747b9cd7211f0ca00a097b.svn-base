.. article:: apps_demo_msp_freertos_lamp
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>; Zdenek Vasicek <vasicek AT fit.vutbr.cz>
    :updated: 20100321

    Ovládání svítidla pomocí RT aplikace 

===============================================
FreeRTOS (ovládání svítidla pomocí RT aplikace)
===============================================

  .. note:: Více o FreeRTOS je mono nalézt na www.freertos.org

.. figure:: free_rtos.png
   :align: center

.. contents:: Obsah

Popis aplikace
==================

S vyuitím prostøedkù RT jádra FreeRTOS  
je pro MSP430 na FITkitu v jazyce C implementována jednoduchá RT aplikace (``Svítidlo`` - CZ, ``Lamp`` - EN)
tvoøená 

1. ``tlaèítkem`` (pro jednoduchost je vyuito tlaèítko ``A`` z maticové klávesnice FITkitu), 

2. ``øídicí èástí`` (implementovanou v MSP430 pomocí prostøedkù FreeRTOS) reagující na stisk tlaèítka a 

3. ``øízenou èástí`` reprezentovanou pro jednoduchost LED diodou ``D5`` (zelená LED umístìná v levém dolním rohu FITkitu).

Aplikaci je mono slovnì ``specifikovat`` následovnì. Svítidlo se mùe nacházet v jednom ze 3 stavù:

1. ``OFF`` (zhasnuto), 

2. ``LOW`` (svítí slabì), 

3. ``BRIGHT`` (svítí silnì). 

Poèáteèní stav svítidla je stav ``OFF``.
Po (prvním) stisku tlaèítka zaène svítidlo svítit slabì (tj. pøejde do stavu ``LOW``).  
Bude-li tlaèítko stisknuto do 500 ms po pøechodu do stavu ``LOW``, pak zaène svítidlo svítit silnì (tj. pøejde do stavu ``BRIGHT``). 
Svítidlo zhasne (tj. pøejde do stavu ``OFF``) bude-li tlaèítko stisknuto a) za více ne 500 ms od pøechodu do stavu ``LOW`` nebo b) pøi silném svitu svítidla (tj. ve stavu ``BRIGHT``). 

.. figure:: lamp_small.png
   :align: center

   Model chování slovnì specifikované aplikace pomocí dvojice komunikujících ``èasovanıch automatù`` (``Timed Automaton``, ``TA``).
   Pozn.: ``y`` je promìnná typu ``clock`` urèená k odmìøování èasu, ``press`` je kanál, pøes kterı automaty komunikují 
   (``press?`` pøedstavuje èekání na pøíchod zprávy po kanálu ``press`` zatímco 
   ``press!`` pøedstavuje vyslání zprávy po kanálu ``press``)

.   
  .. note:: Práci s TA lze vyzkoušet napø. pomocí volnì dostupného nástroje ``UPPAAL`` - viz. ``www.uppaal.com``

Realizace aplikace pomocí prostøedkù RT jádra FreeRTOS
======================================================
  .. note:: ``RT jádro`` se èasto oznaèuje pojmem ``real-time operaèní systém``, popø. ``operaèní systém pro práci v reálném èase`` apod., obvyklá zkratka je ``RTOS``

Aby bylo moné realizovat danou aplikaci pomocí prostøedkù RT jádra,
je nutné zamyslet se nad rozvrením dílèích èástí aplikace do mnoiny tzv. ``RT úloh``.
Konkrétní realizace, tedy pomocí FreeRTOS, mùe mít tuto podobu:

1. stav tlaèítka 'A' je testován úlohou ``keyboardTask``,
která je volána s periodou ``1 ms``. 
Stisk tlaèítka je signalizován nastavením globální promìnné 
``press`` na hodnotu 1. Toto øešení má své nevıhody 
(napø. aktivní èekání úlohy na pøíkazu ``while(!pressed);``), ale je jednoduché
a pro naše úèely postaèuje. Kostra úlohy:

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

2. reakci na stisk tlaèítka 'A' v závislosti na stavu aplikace zajišuje úloha 
``lampTask``. V jednotlivıch stavech úlohy se mìní hodnota promìnné
``lightIntensity`` urèující intenzitu svitu svítidla.
Kostra úlohy je níe:

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

3. svit svítidla je øešen v obsluze pøerušení od èasovaèe B (TIMERB).
Podle aktuální hodnoty promìnné ``lightIntensity`` zajistí
èasovaè odpovídající svit LED diody ``D5``:

::

  interrupt (TIMERB0_VECTOR) Timer_B (void)
  {
    cnt++;
  
    if((cnt>0) && (cnt<=lightIntensity)) { set_led_d5(1); } /* D5 on-time */
    else { set_led_d5(0);}  /* D5 off-time */
  
    if(cnt>=(MAX_LIGHT_LEVELS)) cnt=0;
  
    TBCCR0 = 0x1000;          /* interrupt each x ticks of clk source */
  }


Pøíkazová øádka aplikace
=========================

O pøípadnou obsluhu pøíkazové øádky se stará úloha ``terminalTask``,
která je volána s periodou 1 s:

::

  static void terminalTask(void *param) 
  {
    for (;;) {
      terminal_idle();
      vTaskDelay( 1000 / portTICK_RATE_MS); /* delay for 1000 ms */ 
    }
  }

Inicializace a spuštìní aplikace
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

Zdrojové kódy
=============

Kompletní zdrojové kódy je moné nalézt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovoznìní aplikace
========================
1. pøelote aplikaci

2. naprogramujte MCU a FPGA a spuste terminálovı program.

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.

