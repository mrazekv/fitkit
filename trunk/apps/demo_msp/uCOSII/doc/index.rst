.. article:: apps_demo_msp_ucosii
    :author: Karel Koranda <xkoran01 AT stud.fit.vutbr.cz>; Vladimír Brùžek <xbruze01 AT stud.fit.vutbr.cz>; Tomáš Jílek <xjilek02 AT stud.fit.vutbr.cz>
    :updated: 20101127

    Portace uC/OS-II na platformu FITKit

===============================================
Portace uC/OS-II na platformu FITKit
===============================================

  .. note:: Více o uC/OS-II je možno nalézt na http://www.micrium.com/page/products/rtos/os-ii

.. figure:: ucosii.png
   :align: center

.. contents:: Obsah

uC/OS-II
==================
uC/OS-II je jádro preemptivního real-time multitasking operaèního systému 
založeného na prioritách probíhajících procesù. Je vytvoøen spoleèností Micros a je
poskytován zdarma pro výukové úèely. Je pøedevším urèen pro vestavìné
systémy. Protože se jedná o operaèní systém schopný bìžet na rùzných mikroprocesorových
platformách, je pro jeho zprovoznìní nutné provést portaci pro konkrétní mikrokontroler.

Portace uC/OS-II
==================
K portaci na konkrétní platformu slouží tzv. procesorovì specifické zdrojové
soubory (os_cpu.h, os_cpu_c.c a os_cpu_a.s). Ty je nutné upravit pro konkrétní
mikrokontroler pøedevším z dùvodu jejich rozdílné instrukèní sady a složení
registrù.

OS_CPU.H
==================
Hlavièkový soubor ``os_cpu.h`` obsahuje procesorovì a implementaènì specifické konstanty (definované direktivami 
``#define``), makra a definice datových typù (``typedef``). Datové typy jsou definovány
pøedevším z dùvodu jejich rùzných délek na rùzných platformách.

::

  typedef unsigned char  BOOLEAN;
  typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity  */
  typedef signed   char  INT8S;                    /* Signed    8 bit quantity  */
  typedef unsigned int   INT16U;                   /* Unsigned 16 bit quantity  */
  typedef signed   int   INT16S;                   /* Signed   16 bit quantity  */
  typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity  */
  typedef signed   long  INT32S;                   /* Signed   32 bit quantity  */
  typedef float          FP32;                     /* Single precision floating point  */
  typedef double         FP64;                     /* Double precision floating point  */
  
  typedef unsigned int   OS_STK;                   /* Each stack entry is 16-bit wide  */
  typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (SR = 16 bits)  */

Dále tento soubor obsahuje další velice významnou definici následujících maker:

::

  #define  OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())    /* Disable interrupts */
  #define  OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))    /* Enable  interrupts */

Jedná se o makra umožòující zablokování a odblokování systému pøerušení pøi vstupu
do kritické sekce, nebo pøerušení pøi výskytu v kritické sekci není žádoucí. Proto
každý vstup do kritické sekce by mìl být uvozen voláním ``OS_ENTER_CRITICAL()``
a ukonèen ``OS_EXIT_CRITICAL()``.

Protože však pouhé zablokování a odblokování pøerušení mùže zpùsobit jejich nežádoucí
odblokování (pøed vstupem do kritické sekce byla pøerušení zablokována, ``OS_EXIT_CRITICAL()``
však mùže provést jejich nechtìné odblokování), je vhodnìjším øešením pøi volání ``OS_ENTER_CRITICAL()`` 
uložit obsah registru SR (Status Register), zablokovat pøerušení a pøi opuštìní kritické
sekce obnovit stav pøerušení na základì uloženého SR registru.

Hlavièkový soubor ``os_cpu.h`` obsahuje také neménì dùležitou konstantu OS_STK_GROWTH,
která urèuje, jakým zpùsobem budou data ukládána na zásobník. V pøípadì nastavení této
konstanty na 0, bude zápis do pamìti provádìn od nižších po vyšší adresy, pokud bude nastavena
na 1, pak naopak. Pro potøeby MSP430 je použito ``#define OS_STK_GROWTH 1``.

``OS_TASK_SW()`` je makro vyvolávající pøepnutí kontextu. Pøi pøepnutí kontextu
je potøeba uložit registry pozastaveného procesu a obnovení registrù procesu,
který bude následnì probíhat. Je dùležité také vìdìt, že pøepnutí kontextu
se chová obdobnì jako pøerušení. Pokud tedy cílová architektura podporuje softwarová
pøerušení, je možno ``OS_TASK_SW()`` vyvolat tímto zpùsobem. V takovém pøípadì
se ve vektoru pøerušení musí nacházet procedura ``OSCtxSw()`` definovaná v ``os_cpu_a.s``.
Pro platformy nepodporující softwarová pøerušení a náš pøípad, je možné pouze definovat
``OS_TASK_SW()`` jako volání procedury ``OSCtxSw()``:

::

  #define OS_TASK_SW()    OSCtxSw()

OS_CPU_A.S
==================
V systému uC/OS-II je zapotøebí nadefinovat v asembleru odpovídajícímu konkrétnímu
mikroprocesoru ètyøi funkce - ``OSStartHighRdy()``, ``OSCtxSw()``, ``OSIntCtxSw()``,
 ``OSTickISR()``. Dále jsou v tomto souboru definovány tìla funkcí ``OSCPUSaveSR()``
a ``OSCPURestoreSR()``, které jsou volány již vysvìtlenými makry ``OS_ENTER_CRITICAL()`` 
a ``OS_EXIT_CRITICAL()``.

``OSStartHighRdy()`` pøipraví ke spuštìní úlohu s nejvyšší prioritou pøi startu OS. 
Tato funkce musí zavolat ``OSTaskSwHook()`` (probíhá de facto pøepnutí kontextu),
musí nastavit ``OSRunning`` na ``TRUE``, identifikující bììh jádra, a samozøejmì 
pøepnout na spouštìný proces naètením odpovídajícího zásobníku (registr R1 je
na MSP430 Stack Pointer).

::

  .global OSStartHighRdy
  .type OSStartHighRdy,@function
  OSStartHighRdy:
      CALL       #OSTaskSwHook
      MOV.B       #1, &OSRunning         ; kernel running
  
      MOV.W      r1, &OSISRStkPtr        ; save interrupt stack
  
      MOV.W      &OSTCBHighRdy, R13      ; load highest ready task stack
      MOV.W      @R13, r1
  
      POPALL
  
      RETI                               ; emulate return from interrupt

``OSCtxSw()`` je funkcí umožòující pøepnutí kontextu. Její funkce již byla vysvìtlena
spolu s ``OS_TASK_SW()``. Pøi pøepnutí kontextu je však potøeba uložit registr R2
(Status Register) a obecné registry R4 až R15 na zásobník, uložit souèasný Stack Pointer
(registr R1) do tabulky TCB, zavolat ``OSTaskSwHook()``, protože probíhá pøepnutí kontextu,
vybrat proces s vyšší prioritou a obnovit jeho registry uložené na jeho zásobníku.

::

  .global OSCtxSw
  .type OSCtxSw, @function
  OSCtxSw:
      PUSH.W      r2
  
      PUSHALL
  
      MOV.W      &OSTCBCur, R13          ; OSTCBCur->OSTCBStkPtr = SP
      MOV.W      r1, 0(R13)
  
      CALL       #OSTaskSwHook
  
      MOV.B       &OSPrioHighRdy, R13    ; OSPrioCur = OSPrioHighRdy
      MOV.B       R13, &OSPrioCur
  
      MOV.W      &OSTCBHighRdy, R13      ; OSTCBCur  = OSTCBHighRdy
      MOV.W      R13, &OSTCBCur
  
      MOV.W      @R13, r1                ; SP        = OSTCBHighRdy->OSTCBStkPtr
  
      POPALL
  
      RETI                               ; Return from interrupt.

``OSIntCtxSw()`` je funkcí zpùsobující pøepnutí kontextu z obsluhy pøerušení. Chová 
se obdobnì jako ``OSCtxSw()`` s tím rozdílem, že neukládá souèasný kontext.

::

  .global OSIntCtxSw
  .type OSIntCtxSw, @function
  OSIntCtxSw:
      CALL       #OSTaskSwHook
  
      MOV.B       &OSPrioHighRdy, R13    ; OSPrioCur = OSPrioHighRdy
      MOV.B       R13, &OSPrioCur
  
      MOV.W      &OSTCBHighRdy, R13      ; OSTCBCur  = OSTCBHighRdy
      MOV.W      R13, &OSTCBCur
  
      MOV.W      @R13, r1                ; SP        = OSTCBHighRdy->OSTCBStkPtr
  
      POPALL
  
      RETI                               ; return from interrupt

Funkce ``OSTickISR()`` je jednou z esenciálních souèástí uC/OS-II, nebo je potøeba
pro funkènost èasovaèù a zpoždìní v systému. Jedná se o obsluhu pøerušení vyvolaném
hardwarovým èasovaèem (Watchdog Timer).

Protože se jedná o obsluhu pøerušení, je potøeba na zaèátku uložit všechny obecné
registry R4 až R15 na zásobník, vyøadit èasové pøerušení. Pokud nedošlo k žádnému
dalšímu pøerušení, uložíme SP souèasné úlohy a naèteme SP pro obsluhu tohoto pøerušení.
Dále je potøeba OS informovat o tom, že k pøerušení došlo - k tomu slouží globální
promìnná ``OSIntNesting``. Následuje opìtovné povolení èasového a obecných pøerušení.
Poté je volána funkce ``OSTimeTick``, což je obslužná rutina pro zpracování hodinového
tiknutí. Po jejím prùbìhu je zapotøebí opìt vyøadit obecná pøerušení a zavolat ``OSIntExit()``,
která mimo jiné sníží ``OSIntNesting``. Pokud se tato promìnná nsálednì rovná nule,
všechna pøerušení byla obsloužena a my mùžeme obnovit SP pùvodní úlohy. Nakonec probìhne
obnova obecných registrù R4 až R15 a návrat z obsluhy pøerušení.

::

  .global vector_fff4
  vector_fff4:                           ; wd timer ISR
      PUSHALL
      
      BIC.B       #0x01, &0              ; disable wd timer interrupt
  
      CMP.B       #0, &OSIntNesting      ; if (OSIntNesting == 0)
      JNE         WDT_ISR_1
  
      MOV.W      &OSTCBCur, R13          ;     save task stack
      MOV.W      r1, 0(R13)
  
      MOV.W      &OSISRStkPtr, r1        ;     load interrupt stack
  
  WDT_ISR_1:
      INC.B       &OSIntNesting          ; increase OSIntNesting
  
      BIS.B       #0x01, &0              ; enable wd timer interrupt
  
      EINT                               ; enable general interrupt to allow for interrupt nesting
  
      CALL       #OSTimeTick             ; call ticks routine
  
      DINT                               ; IMPORTANT: disable general interrupt BEFORE calling OSIntExit()
  
      CALL       #OSIntExit
  
      CMP.B       #0, &OSIntNesting      ; if (OSIntNesting == 0)
      JNE         WDT_ISR_2
  
      MOV.W      &OSTCBHighRdy, R13      ;     restore task stack SP
      MOV.W      @R13, r1
  
  WDT_ISR_2:
      POPALL
  
      RETI                               ; return from interrupt

Funkci nemáme pojmenovanou jako ``OSTickISR()``, ale ``vector_fff4``. To je
z toho dùvodu, že MSP430 má na adrese ``FFF4h`` uloženu právì obsluhu pøerušení
od Watchdog èasovaèe.

Funkce ``OSCPUSaveSR()`` a ``OSCPURestoreSR()`` jsou již popsané metody pro uložení
Status Registru a vyøazení/obnovení systému pøerušení pøi vstupu do kritických sekcí.
Pøedpokládáme, že pøes R15 bude pøedáván výsledek funkce ``OSCPUSaveSR()`` a pøijímán
parametr funkce ``OSCPURestoreSR()``.

::

  .global OSCPUSaveSR
  .type OSCPUSaveSR, @function
  OSCPUSaveSR:
      MOV.W       r2, R15
      DINT
      RET
  
  .global OSCPURestoreSR
  .type OSCPURestoreSR, @function
  OSCPURestoreSR:
      MOV.W       R15, r2
      RET

OS_CPU_C.C
==================
Soubor ``os_cpu_c.c`` obsahuje definice funkcí ``OSTaskStkInit()``, ``OSTaskCreateHook()``,
``OSTaskDelHook()``, ``OSTaskSwHook()``, ``OSTaskStatHook()`` a ``OSTimeTickHook()``. Nicménì
pro funkènost uC/OS-II je nezbytnì nutné implementovat pouze jednu z nich - ``OSTaskStkInit()``.

``OSTaskStkInit()`` je funkcí, která inicializuje zásobník pøi vytváøení úlohy a to tak,
že simuluje chování "jako by právì nastalo pøerušení" - na zásobníku je uložena 
adresa kódu úlohy, Status Registr a obecné registry R4 až R15. Pøedpokládáme, že pøes 
R15 je pøedáván parametr úlohy. 

::

  OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
  {
      INT16U  *top;
  
      opt    = opt;
      top    = (INT16U *)ptos;
      top--;
  
      *top-- = (INT16U)(task);                          /* Interrupt return pointer */
      *top-- = (INT16U)0x0008;                          /* Status register          */
      *top-- = (INT16U)(p_arg);                         /* Pass 'p_arg' through register R15 */
      *top-- = (INT16U)0x1414;
      *top-- = (INT16U)0x1313;
      *top-- = (INT16U)0x1212;
      *top-- = (INT16U)0x1111;
      *top-- = (INT16U)0x1010;
      *top-- = (INT16U)0x0909;
      *top-- = (INT16U)0x0808;
      *top-- = (INT16U)0x0707;
      *top-- = (INT16U)0x0606;
      *top-- = (INT16U)0x0505;
      *top   = (INT16U)0x0404;
  
       return ((OS_STK *)top);  
  }

Ostatní funkce definované v tomto souboru slouží pro potøeby uživatelù uC/OS-II
rozšíøit funkcionalitu OS, jejich tìla tedy mohou zùstat prázdná. Pro naše potøeby
je nebylo tøeba více rozvíjet, pøesto zde však popíšeme, kdy je jaká funkce volána.

``OSTaskCreateHook()`` je volána kdykoli je vytvoøena nová úloha. Je volána již po
vytvoøení PCB, avšak ještì pøed spuštìním plánovaèe úloh. Taktéž je ještì vyøazen 
systém pøerušení. Jako parametr je této funkci pøedán ukazatel na PCB vytváøené úlohy.
Tato funkce je generována pouze v pøípadì, že je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavièkovém souboru ``os_cfg.h``.

``OSTaskDelHook()`` je volána kdykoli je úloha odstranìna ze systému. Obdobnì jako
``OSTaskCreateHook()``, i tato funkce obdrží jako parametr ukazatel na PCB právì
odstraòované úlohy. Tato funkce je generována pouze v pøípadì, že je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavièkovém souboru ``os_cfg.h``.

``OSTaskSwHook()`` je volána vždy, když dochází k pøepnutí kontextu. Tato funkce
má pøístup ke globálním promìnným obsahujícím ukazatele na PCB aktuální úlohy a úlohy,
na kterou se kontext pøepíná. Je jisté, že pøi volání této funkce jsou vždy vyøazena
pøerušení. Tato funkce je generována pouze v pøípadì, že je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavièkovém souboru ``os_cfg.h``.

``OSTaskStatHook()`` je volána každou sekundu funkcí ``OSTaskStat()`` (statistická funkce),
èímž lze rozšíøit její funkcionalitu. Tato funkce je generována pouze v pøípadì, že je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavièkovém souboru ``os_cfg.h``.

``OSTimeTickHook()`` je volána funkcí ``OSTimeTick()`` pøi každém sytémovém tiknutí, resp.
tìsnì pøed ním. Tato funkce je generována pouze v pøípadì, že je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavièkovém souboru ``os_cfg.h``.

Statistická úloha
==================
Statistická úloha je úloha OS, která umožòuje shromažïování statistik bìhu operaèního systému,
resp. využití mikroprocesoru. Tato hodnota je uložena v promìnné ``OSCPUUsage``, jednotkou
jsou procenta.

Pokud je statistická úloha využita v systému, je potøeba, aby byla vytvoøena voláním
funkce ``OSStatInit()`` pouze v jedné a vždy první úloze OS. Tato úloha bývá oznaèována
jako "startovací úloha" a v jejím tìle jsou kromì statistické úlohy a její samotné funkènosti
vytvoøeny také další úlohy. Ukázka, jak je volána, bude nastínìna v popisu vytvoøené ukázkové 
aplikace nad uC/OS-II.

Na tomto místì je nutné podotknout, že statistická úloha je generována pouze v pøípadì,
že je konstanta OS_TASK_STAT_EN nastavena na 1. Dále je nutné zmínit, že na mikrokontroleru
MSP430F168, což odpovídá verzi FITKit 1.2, se nepodaøilo statistickou úlohu zprovoznit.
Protože však MSP430F168 obsahuje velice malou programovou pamì a není reálné zprovoznit
nìjakou výraznì užiteènou a rozsáhlou aplikaci nad uC/OS-II, tímto problémem se nebudeme 
dále zabývat.

Ukázková aplikace nad uC/OS-II
================================
Protože se podaøilo rozchodit uC/OS-II na platformách FITKit 1.2 i 2.0 (aè pro 1.2 
s jistými omezeními), jsou pro jednotlivé verze FITKitu aplikace rozdílné. Kód aplikace
se nachází v souboru ``app.c``. Protože pøekladový systém QDevKit neumí na základì
verze FITKitu vybrat soubory, jsou obì aplikace definovány v tomto souboru a direktivami
``#if`` je pøi pøekladu vybírána správná aplikace pro konkrétní platformu. To sice vede
k ménì pøehlednému kódu, zachováme však funkènost obou dvou verzí platformy FITKit.

Ukázka pro FITKit 1.2
======================
Tuto aplikaci implementovali Ing. Jiøí Novotòák a Ing. Tomáš Novotný v rámci projektu
do pøedmìtu ROS. Tuto aplikaci jsme se tedy rozhodli ponechat pro verzi FITKitu 1.2, 
nebo, jak jsme již zmínili, jiná, složitìjší aplikace vyvinout pro MSP430F168 nejde,
pro ukázku funkènosti uC/OS-II však postaèuje.

Aplikace zpracovává dvì úlohy - jedna bliká zelenou diodou D5, druhá èervenou. Obì
diody blikají s rùznou frekvencí a èasovými rozestupy.

::

  /*
   * Uloha blikajici se zelenou LED diodou (D5)
   * Kazdych 100ms prepina stav diody
   */
  void green_task(void *data) {
     P1OUT &= ~BIT0;
  
     for(;;) {
        OSTimeDlyHMSM(0,0,0,100);
        P1OUT ^= BIT0;
     }
  }
  
  /*
   * Uloha blikajici s cervenou LED diodou (D6)
   * Kazdou sekundu blikne na 100ms
   */
  void red_task(void *data) {
     P5OUT |= BIT0;
     for(;;) {
        OSTimeDlyHMSM(0,0,0,900);
        P5OUT &= ~BIT7;
        OSTimeDlyHMSM(0,0,0,100);
        P5OUT |= BIT7;
     }
  }

Pro inicializaci hardwaru (pøedevším clock system) je napsána funkce ``my_initialize_hardware()``,
která je inspirovaná kódem z knihovny ``libfitkit``.

Funkce ``main()`` programu vypadá pro tuto aplikaci následovnì:

::

  int main() {
  
     // Inicializace HW
     my_initialize_hardware();
  
     // Smer pinu diod
     P1DIR |= BIT0; // D5
     P5DIR |= BIT7; // D6
  
     // Vypnuti diod
     P1OUT |= BIT0;
     P5OUT |= BIT7;
    
     // Inicializace OS a uloh
      OSInit();
  
      // Uloha blikani zelene diody
      OSTaskCreate(green_task, (void *)0, (void *)&TaskGreenLEDStk[TASK_STK_SIZE - 1], 0);
      // Uloha blikani cervene diody
      OSTaskCreate(red_task, (void *)0, (void *)&TaskRedLEDStk[TASK_STK_SIZE - 1], 1);
  
      // Spusteni watchdogu
      WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL; // Timer mode, predeleni 32768
      IE1 |= WDTIE;
   
      // Povoleni general interrupt
      _EINT();
   
      // Start multitaskingu
      OSStart();
  
     return 0;
  }
  
Z hlediska uC/OS-II jsou dùležitá volání ``OSInit()``, ``OSTaskCreate()`` a ``OSStart()``.
``OSInit()`` provede inicializaci operaèního systému a jeho základních úloh. Voláním 
``OSTaskCreate()`` jsou do systému pøidány úlohy blikání zelené a èervené diody. 
``OSStart()`` pak provede spuštìní multitaskingu. Je dùležité, aby pøed spuštìním
multitaskingu byla povolena obecná pøerušení a spuštìn Watchdog èasovaè! Pokud tomu
tak nebude, pak uC/OS-II nebude fungovat správnì.

Ukázka pro FITKit 2.0
======================
Pro vytvoøení ukázkové aplikace na FITKit 2.0 jsme se rozhodli využít knihovny
``libfitkit``, což na FITKitu 1.2 nebylo možné z dùvodu malé kapacity MSP430F168.
Ukázková aplikace tedy bude schopna zasílat zprávy na terminál, reagovat na stisk
urèitých kláves, ovládat svícení diod D5 a D6 a ovládat výpis na displeji. Navíc,
pokud je v souboru ``os_cfg.h`` nastavena konstanta OS_TASK_STAT_EN na 1, bude
spuštìna statistická funkce a po stisku urèité klávesy budou údaje o zatížení
mikroprocesoru vypisovány na displej/terminál. Pøi vytváøení aplikace jsme
se èásteènì inspirovali u ukázkové aplikace nad FreeRTOS.

Stav, v jakém se aplikace nachází, budeme ukládat do globálních promìnných:

::
  
  volatile char last_ch;
  unsigned char c_pressed = 0;            // priznak stisku klavesy 'C'
  unsigned char d_pressed = 0;            // priznak stisku klavesy 'D'
  unsigned char load_pressed = 0;         // priznak stisku klavesy '*'
  unsigned char previous_c_pressed = 0;   // priznak zpracovaneho stisku klavesy 'C'
  
  unsigned char light_red = 1;            // priznak stisku klavesy 'A' - dioda D6
  unsigned char light_green = 1;          // priznak stisku klavesy 'B' - dioda D5

První úlohou, kterou bude OS zpracovávat, bude práce s klávesnicí.  Ta bude v pøípadì
stisku klávesy 'A' rozsvìcet/zhasínat èervenou diodu D6 a odesílat o tom zprávu na terminál,
pøi stisku klávesy 'B' rozsvìcet/zhasínat zelenou diodu D5 a odesílat o tom zprávu na terminál.
Pøi stisku klávesy 'C' bude nastavován pøíznak jejího stisknutí a bude o tom odeslána
zpráva na terminál - klávesa 'C' bude ovládat zapnutí/vypnutí displeje.
Pøi stisku klávesy 'D' bude vybíráno, zda na displej bude vypisován údaj
o rozsvícených/zhasnutých diodách nebo zda bude vypisována zátìž mikroprocesoru
vypoèítaná statistickou úlohou (pokud je generována). Pøi stisku klávesy '*' bude
nastaven pøíznak generování zátìže v rámci zátìžové úlohy.

::

  void keyboardTask(void *param) 
  {
    param = param;
    char ch;
  
    last_ch = 0;
    keyboard_init(); // inicializace klavesnice
  
    for (;;) 
    {
      ch = key_decode(read_word_keyboard_4x4()); // dekodovani znaku z klavesnice
      if (ch != last_ch) {
        last_ch = ch;
        if (ch != 0) {
           switch (ch) {
             case 'A': // stisk klavesy 'A'
               set_led_d6(light_red); // nastaveni rozsviceni/zhasnuti diody
               if (light_red == 1)
               { // dioda byla rozsvicena
                  term_send_str_crlf("'A': Turn on red light D6.");
               } else { // dioda byla zhasnuta
                  term_send_str_crlf("'A': Turn off red light D6.");
               }
               // nastaveni pristiho stavu diody
               light_red = (light_red == 0 ? 1 : 0); 
               break;
            case 'B': // stisk klavesy 'B'
               set_led_d5(light_green); // rozsviceni/zhasnuti diody
               if (light_green == 1)
               { // dioda je rozsvicena
                  term_send_str_crlf("'B': Turn on green light D5.");
               } else { // dioda je zhasnuta
                  term_send_str_crlf("'B': Turn off green light D5.");
               }
               // nastaveni pristiho stavu diody
               light_green = (light_green == 0 ? 1 : 0);
               break;
             case 'C': // stisk klavesy 'C'
               c_pressed = (c_pressed == 0 ? 1 : 0); // nastaveni priznaku
               if (c_pressed) { // stisknuta klavesa -> vypnuty displej
                  term_send_str_crlf("'C': Display OFF");
               } else { // spusteny displej
                  term_send_str_crlf("'C': Display ON");
               }
               break;
             case 'D':
               d_pressed = (d_pressed == 0 ? 1 : 0); // nastaveni priznaku
               if (d_pressed) { // stisknuta klavesa -> vypis statistik
                  term_send_str_crlf("'D': CPU Usage Monitoring ON");
               } else { // vypnuty vypis statistik
                  term_send_str_crlf("'D': CPU Usage Monitoring OFF");
               }
               break;
             case '*':
               load_pressed = (load_pressed == 0 ? 1 : 0); // nastaveni priznaku
               if (load_pressed) { // stisknuta klavesa -> zatezova uloha
                  term_send_str_crlf("'*': Load task ON");
               } else { // vypnuta zatezova uloha
                  term_send_str_crlf("'*': Load task OFF");
               }
               break;
             default:
               break;
           }
        }
     }
     OSTimeDlyHMSM(0,0,0,100); // zpozdeni 100ms
    }
  }

Druhá úloha bude mít za úkol ovládání displeje na základì pøíznakù stisknutých kláves.
Na základì stisku kláves/stavu diod bude na displej implicitnì vypisovat, v jakém
stavu se diody nachází (napø. D5:OFF D6:ON). V pøípadì, že je nastaven pøíznak
po stisknutí klávesy 'C', displej je považován za vypnutý a nevypisuje se na nìj nic.
V pøípadì, že je nastaven pøíznak klávesou 'D', a není displej vypnutý klávesou 'C',
je na displej místo údajù o diodách vypisován stav zatížení mikroprocesoru (napø. 
CPU 98%). V pøípadì, že je nastaven pøíznak klávesy '*', je øízení displeje
ponecháno na zátìžové úloze ``loadTask()``.

::

  void displayTask(void *param)
  {
     LCD_init(); // inicializace LCD
     
     for (;;)
     {
     
        if (d_pressed) { // nastaven priznak 'D'
  #if OS_TASK_STAT_EN // je generovana statisticka uloha
          char result[20];
          sprintf (result, "CPU %d%c", OSCPUUsage, '\%'); // format vystupu stat.funkce
          if (!c_pressed && !load_pressed) { // pokud neni priznak 'C' a '*'
            if (previous_c_pressed) previous_c_pressed = 0;
            LCD_write_string(result); // vypis na displej
          }
          term_send_str_crlf(result); // odeslani dat na terminal
  #else
          if (!c_pressed && !load_pressed) {
            if (previous_c_pressed) previous_c_pressed = 0;
            LCD_write_string("STATS DISABLED");
          }
          term_send_str_crlf("STATS DISABLED");
  #endif
          OSTimeDlyHMSM(0,0,1,0);
        } else { // neni nastaven 'D'
          if (!c_pressed && !load_pressed) { // neni nastaven 'C' ani '*'
            if (previous_c_pressed) previous_c_pressed = 0;
            // vypis udaju o diodach
            if (light_red == 0 && light_green == 0) {
                LCD_write_string("D6:ON D5:ON");
              } else if (light_red == 1 && light_green == 0) {
                LCD_write_string("D6:OFF D5:ON");
              } else if (light_red == 1 && light_green == 1) {
                LCD_write_string("D6:OFF D5:OFF");
              } else {
                LCD_write_string("D6:ON D5:OFF");
              }
              OSTimeDlyHMSM(0,0,0,500); // zpozdeni 0.5s
          }
          if (load_pressed) // je nastaven priznak '*' zatezove ulohy
          { // tato uloha nebude nyni obsluhovat displej
            OSTimeDlyHMSM(0,0,0,500); // zpozdeni 0.5s
          }
        }
        if (c_pressed)
        { // je nastaven priznak vypnuteho displeje (klavesa 'C')
          if (previous_c_pressed == 0) // pokud jsme nastaveni priznaku identifikovali
          { // poprve
            LCD_write_string(" "); // cisteni displeje
            previous_c_pressed = 1; // priste uz cistit nebudeme, setrime
          }
          OSTimeDlyHMSM(0,0,0,500); // zpozdeni 500ms
        }
     } 
  }

Tøetí úloha, zátìžová úloha ``loadTask()``, je v aplikaci pro demonstraci
funkènosti statistické funkce. Tato úloha, pokud je nastaven pøíslušný pøíznak,
generuje zátìž mikroèipu rychlým obnovováním textu na displeji (to je èinìno každých
50ms).

::

  void loadTask(void *param)
  {
    for (;;)
    {
      while (load_pressed) // dokud je nastaven priznak stisknute '*'
      {
        if (!c_pressed) { // pokud neni vypnuty displej
          if (previous_c_pressed) previous_c_pressed = 0; // pokud je treba, prehod stav displeje
  #if OS_TASK_STAT_EN // je generovana statisticka funkce
          char result[32];
          sprintf(result, "!!!LOAD TASK!!! CPU %d%c", OSCPUUsage, '\%');
          LCD_write_string(result);
  #else // neni generovana statisticka funkce
          LCD_write_string("!!!LOAD TASK!!! STATS DISABLED");
  #endif
        }
        OSTimeDlyHMSM(0,0,0,50); // zpozdeni 50ms
      }
      OSTimeDlyHMSM(0,0,1,0); // zpozdeni 1s
    }
  }


V aplikaci je ještì definována tøetí úloha. Tato úloha je oznaèena jako ``startTask()``
a je v ní, pokud je to vyžadováno, inicializována statistická funkce. V tìle této funkce
jsou také inicializovány ostatní úlohy. Samotná úloha jinak nic neprovádí.

`Pozn.: Maximum úloh v aplikaci se nastavuje v souboru` ``os_cfg.h`` `konstantou`
``OS_MAX_TASKS``.

::

  void startTask(void *param)
  {
  #if OS_TASK_STAT_EN
     OSStatInit(); // inicializace statisticke ulohy
  #endif
  
    OSTaskCreate(keyboardTask, (void *)0, (void *)&TaskKeyStk[TASK_STK_SIZE - 1], 1);     // uloha zpracovani klavesnice
    OSTaskCreate(displayTask, (void *)0, (void *)&TaskDisplayStk[TASK_STK_SIZE - 1], 2);  // uloha zpracovani displeje
    OSTaskCreate(loadTask, (void *)0, (void *)&TaskLoadStk[TASK_STK_SIZE - 1], 3);        // zatezova uloha
  
    for (;;) {
      OSTimeDlyHMSM(0,0,1,0); // zpozdeni 1s
    }
  
  }
  
Funkce ``main()`` celé aplikace pak vypadá takto:

::

  int main() {
  
     // Inicializace HW
     initialize_hardware();
  
     // Inicializace OS a uloh
      OSInit();
  
      OSTaskCreate(startTask, (void *)0, (void*)&TaskStartStk[TASK_STK_SIZE - 1], 0);
  
      // Spusteni watchdogu
      WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL; // Timer mode, predeleni 32768
      IE1 |= WDTIE;
   
      // Povoleni general interrupt
      _EINT();
   
      // Start multitaskingu
      OSStart();
  
     return 0;
  }
  
Inicializace HW je ponechána v tomto pøípadì na funkci ``initialize_hardware()``
z knihovny ``libfitkit``. Na této funkci ``main()`` se ale od té pøedchozí pro 
FITKit 1.2 mnoho neliší - jediným podstatným rozdílem je vytvoøení úloh OS až
v rámci úlohy ``startTask()``.

Zprovoznìní aplikace
=====================
Pro správný chod aplikace je nutné mít adresáø se zdrojovými kódy umístìný v 
svn adresáøi FITKitu, v ``/apps/demo_msp``. Pro zprovoznìní aplikace na FITKitu
staèí pøeložit aplikaci, naprogramovat MCU a FPGA a poté spustit terminálový program.

Podìkování
============
Tímto bychom chtìli podìkovat za pomoc a konzultace Ing. Jiøímu Novotòákovi a Ing.
Tomáši Novotnému, kteøí již døíve s uC/OS-II pracovali a poskytli nám k dispozici 
výsledky jejich práce, na které jsme mohli navázat a tím tento projekt rozvinout.
Taktéž dìkujeme Ing. Josefu Strnadelovi, Ph.D., za poskytnuté studijní materiály.

Použitá literatura
===================
Labrosse J. Jean: uC/OS-II Real-Time Kernel

Texas Instruments: MSP430x1xx Family User's Guide

Texas Instruments: MSP430x2xx Family User's Guide
