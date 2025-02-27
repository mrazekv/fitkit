.. article:: apps_demo_msp_ucosii
    :author: Karel Koranda <xkoran01 AT stud.fit.vutbr.cz>; Vladim�r Br��ek <xbruze01 AT stud.fit.vutbr.cz>; Tom� J�lek <xjilek02 AT stud.fit.vutbr.cz>
    :updated: 20101127

    Portace uC/OS-II na platformu FITKit

===============================================
Portace uC/OS-II na platformu FITKit
===============================================

  .. note:: V�ce o uC/OS-II je mo�no nal�zt na http://www.micrium.com/page/products/rtos/os-ii

.. figure:: ucosii.png
   :align: center

.. contents:: Obsah

uC/OS-II
==================
uC/OS-II je j�dro preemptivn�ho real-time multitasking opera�n�ho syst�mu 
zalo�en�ho na priorit�ch prob�haj�c�ch proces�. Je vytvo�en spole�nost� Micros a je
poskytov�n zdarma pro v�ukov� ��ely. Je p�edev��m ur�en pro vestav�n�
syst�my. Proto�e se jedn� o opera�n� syst�m schopn� b�et na r�zn�ch mikroprocesorov�ch
platform�ch, je pro jeho zprovozn�n� nutn� prov�st portaci pro konkr�tn� mikrokontroler.

Portace uC/OS-II
==================
K portaci na konkr�tn� platformu slou�� tzv. procesorov� specifick� zdrojov�
soubory (os_cpu.h, os_cpu_c.c a os_cpu_a.s). Ty je nutn� upravit pro konkr�tn�
mikrokontroler p�edev��m z d�vodu jejich rozd�ln� instruk�n� sady a slo�en�
registr�.

OS_CPU.H
==================
Hlavi�kov� soubor ``os_cpu.h`` obsahuje procesorov� a implementa�n� specifick� konstanty (definovan� direktivami 
``#define``), makra a definice datov�ch typ� (``typedef``). Datov� typy jsou definov�ny
p�edev��m z d�vodu jejich r�zn�ch d�lek na r�zn�ch platform�ch.

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

D�le tento soubor obsahuje dal�� velice v�znamnou definici n�sleduj�c�ch maker:

::

  #define  OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())    /* Disable interrupts */
  #define  OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))    /* Enable  interrupts */

Jedn� se o makra umo��uj�c� zablokov�n� a odblokov�n� syst�mu p�eru�en� p�i vstupu
do kritick� sekce, nebo� p�eru�en� p�i v�skytu v kritick� sekci nen� ��douc�. Proto
ka�d� vstup do kritick� sekce by m�l b�t uvozen vol�n�m ``OS_ENTER_CRITICAL()``
a ukon�en ``OS_EXIT_CRITICAL()``.

Proto�e v�ak pouh� zablokov�n� a odblokov�n� p�eru�en� m��e zp�sobit jejich ne��douc�
odblokov�n� (p�ed vstupem do kritick� sekce byla p�eru�en� zablokov�na, ``OS_EXIT_CRITICAL()``
v�ak m��e prov�st jejich necht�n� odblokov�n�), je vhodn�j��m �e�en�m p�i vol�n� ``OS_ENTER_CRITICAL()`` 
ulo�it obsah registru SR (Status Register), zablokovat p�eru�en� a p�i opu�t�n� kritick�
sekce obnovit stav p�eru�en� na z�klad� ulo�en�ho SR registru.

Hlavi�kov� soubor ``os_cpu.h`` obsahuje tak� nem�n� d�le�itou konstantu OS_STK_GROWTH,
kter� ur�uje, jak�m zp�sobem budou data ukl�d�na na z�sobn�k. V p��pad� nastaven� t�to
konstanty na 0, bude z�pis do pam�ti prov�d�n od ni���ch po vy��� adresy, pokud bude nastavena
na 1, pak naopak. Pro pot�eby MSP430 je pou�ito ``#define OS_STK_GROWTH 1``.

``OS_TASK_SW()`` je makro vyvol�vaj�c� p�epnut� kontextu. P�i p�epnut� kontextu
je pot�eba ulo�it registry pozastaven�ho procesu a obnoven� registr� procesu,
kter� bude n�sledn� prob�hat. Je d�le�it� tak� v�d�t, �e p�epnut� kontextu
se chov� obdobn� jako p�eru�en�. Pokud tedy c�lov� architektura podporuje softwarov�
p�eru�en�, je mo�no ``OS_TASK_SW()`` vyvolat t�mto zp�sobem. V takov�m p��pad�
se ve vektoru p�eru�en� mus� nach�zet procedura ``OSCtxSw()`` definovan� v ``os_cpu_a.s``.
Pro platformy nepodporuj�c� softwarov� p�eru�en� a n� p��pad, je mo�n� pouze definovat
``OS_TASK_SW()`` jako vol�n� procedury ``OSCtxSw()``:

::

  #define OS_TASK_SW()    OSCtxSw()

OS_CPU_A.S
==================
V syst�mu uC/OS-II je zapot�eb� nadefinovat v asembleru odpov�daj�c�mu konkr�tn�mu
mikroprocesoru �ty�i funkce - ``OSStartHighRdy()``, ``OSCtxSw()``, ``OSIntCtxSw()``,
 ``OSTickISR()``. D�le jsou v tomto souboru definov�ny t�la funkc� ``OSCPUSaveSR()``
a ``OSCPURestoreSR()``, kter� jsou vol�ny ji� vysv�tlen�mi makry ``OS_ENTER_CRITICAL()`` 
a ``OS_EXIT_CRITICAL()``.

``OSStartHighRdy()`` p�iprav� ke spu�t�n� �lohu s nejvy��� prioritou p�i startu OS. 
Tato funkce mus� zavolat ``OSTaskSwHook()`` (prob�h� de facto p�epnut� kontextu),
mus� nastavit ``OSRunning`` na ``TRUE``, identifikuj�c� b��h j�dra, a samoz�ejm� 
p�epnout na spou�t�n� proces na�ten�m odpov�daj�c�ho z�sobn�ku (registr R1 je
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

``OSCtxSw()`` je funkc� umo��uj�c� p�epnut� kontextu. Jej� funkce ji� byla vysv�tlena
spolu s ``OS_TASK_SW()``. P�i p�epnut� kontextu je v�ak pot�eba ulo�it registr R2
(Status Register) a obecn� registry R4 a� R15 na z�sobn�k, ulo�it sou�asn� Stack Pointer
(registr R1) do tabulky TCB, zavolat ``OSTaskSwHook()``, proto�e prob�h� p�epnut� kontextu,
vybrat proces s vy��� prioritou a obnovit jeho registry ulo�en� na jeho z�sobn�ku.

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

``OSIntCtxSw()`` je funkc� zp�sobuj�c� p�epnut� kontextu z obsluhy p�eru�en�. Chov� 
se obdobn� jako ``OSCtxSw()`` s t�m rozd�lem, �e neukl�d� sou�asn� kontext.

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

Funkce ``OSTickISR()`` je jednou z esenci�ln�ch sou��st� uC/OS-II, nebo� je pot�eba
pro funk�nost �asova�� a zpo�d�n� v syst�mu. Jedn� se o obsluhu p�eru�en� vyvolan�m
hardwarov�m �asova�em (Watchdog Timer).

Proto�e se jedn� o obsluhu p�eru�en�, je pot�eba na za��tku ulo�it v�echny obecn�
registry R4 a� R15 na z�sobn�k, vy�adit �asov� p�eru�en�. Pokud nedo�lo k ��dn�mu
dal��mu p�eru�en�, ulo��me SP sou�asn� �lohy a na�teme SP pro obsluhu tohoto p�eru�en�.
D�le je pot�eba OS informovat o tom, �e k p�eru�en� do�lo - k tomu slou�� glob�ln�
prom�nn� ``OSIntNesting``. N�sleduje op�tovn� povolen� �asov�ho a obecn�ch p�eru�en�.
Pot� je vol�na funkce ``OSTimeTick``, co� je obslu�n� rutina pro zpracov�n� hodinov�ho
tiknut�. Po jej�m pr�b�hu je zapot�eb� op�t vy�adit obecn� p�eru�en� a zavolat ``OSIntExit()``,
kter� mimo jin� sn�� ``OSIntNesting``. Pokud se tato prom�nn� ns�ledn� rovn� nule,
v�echna p�eru�en� byla obslou�ena a my m��eme obnovit SP p�vodn� �lohy. Nakonec prob�hne
obnova obecn�ch registr� R4 a� R15 a n�vrat z obsluhy p�eru�en�.

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

Funkci nem�me pojmenovanou jako ``OSTickISR()``, ale ``vector_fff4``. To je
z toho d�vodu, �e MSP430 m� na adrese ``FFF4h`` ulo�enu pr�v� obsluhu p�eru�en�
od Watchdog �asova�e.

Funkce ``OSCPUSaveSR()`` a ``OSCPURestoreSR()`` jsou ji� popsan� metody pro ulo�en�
Status Registru a vy�azen�/obnoven� syst�mu p�eru�en� p�i vstupu do kritick�ch sekc�.
P�edpokl�d�me, �e p�es R15 bude p�ed�v�n v�sledek funkce ``OSCPUSaveSR()`` a p�ij�m�n
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
Soubor ``os_cpu_c.c`` obsahuje definice funkc� ``OSTaskStkInit()``, ``OSTaskCreateHook()``,
``OSTaskDelHook()``, ``OSTaskSwHook()``, ``OSTaskStatHook()`` a ``OSTimeTickHook()``. Nicm�n�
pro funk�nost uC/OS-II je nezbytn� nutn� implementovat pouze jednu z nich - ``OSTaskStkInit()``.

``OSTaskStkInit()`` je funkc�, kter� inicializuje z�sobn�k p�i vytv��en� �lohy a to tak,
�e simuluje chov�n� "jako by pr�v� nastalo p�eru�en�" - na z�sobn�ku je ulo�ena 
adresa k�du �lohy, Status Registr a obecn� registry R4 a� R15. P�edpokl�d�me, �e p�es 
R15 je p�ed�v�n parametr �lohy. 

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

Ostatn� funkce definovan� v tomto souboru slou�� pro pot�eby u�ivatel� uC/OS-II
roz���it funkcionalitu OS, jejich t�la tedy mohou z�stat pr�zdn�. Pro na�e pot�eby
je nebylo t�eba v�ce rozv�jet, p�esto zde v�ak pop�eme, kdy je jak� funkce vol�na.

``OSTaskCreateHook()`` je vol�na kdykoli je vytvo�ena nov� �loha. Je vol�na ji� po
vytvo�en� PCB, av�ak je�t� p�ed spu�t�n�m pl�nova�e �loh. Takt� je je�t� vy�azen 
syst�m p�eru�en�. Jako parametr je t�to funkci p�ed�n ukazatel na PCB vytv��en� �lohy.
Tato funkce je generov�na pouze v p��pad�, �e je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavi�kov�m souboru ``os_cfg.h``.

``OSTaskDelHook()`` je vol�na kdykoli je �loha odstran�na ze syst�mu. Obdobn� jako
``OSTaskCreateHook()``, i tato funkce obdr�� jako parametr ukazatel na PCB pr�v�
odstra�ovan� �lohy. Tato funkce je generov�na pouze v p��pad�, �e je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavi�kov�m souboru ``os_cfg.h``.

``OSTaskSwHook()`` je vol�na v�dy, kdy� doch�z� k p�epnut� kontextu. Tato funkce
m� p��stup ke glob�ln�m prom�nn�m obsahuj�c�m ukazatele na PCB aktu�ln� �lohy a �lohy,
na kterou se kontext p�ep�n�. Je jist�, �e p�i vol�n� t�to funkce jsou v�dy vy�azena
p�eru�en�. Tato funkce je generov�na pouze v p��pad�, �e je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavi�kov�m souboru ``os_cfg.h``.

``OSTaskStatHook()`` je vol�na ka�dou sekundu funkc� ``OSTaskStat()`` (statistick� funkce),
��m� lze roz���it jej� funkcionalitu. Tato funkce je generov�na pouze v p��pad�, �e je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavi�kov�m souboru ``os_cfg.h``.

``OSTimeTickHook()`` je vol�na funkc� ``OSTimeTick()`` p�i ka�d�m syt�mov�m tiknut�, resp.
t�sn� p�ed n�m. Tato funkce je generov�na pouze v p��pad�, �e je nastavena konstanta OS_CPU_HOOKS_EN na 1
v hlavi�kov�m souboru ``os_cfg.h``.

Statistick� �loha
==================
Statistick� �loha je �loha OS, kter� umo��uje shroma��ov�n� statistik b�hu opera�n�ho syst�mu,
resp. vyu�it� mikroprocesoru. Tato hodnota je ulo�ena v prom�nn� ``OSCPUUsage``, jednotkou
jsou procenta.

Pokud je statistick� �loha vyu�ita v syst�mu, je pot�eba, aby byla vytvo�ena vol�n�m
funkce ``OSStatInit()`` pouze v jedn� a v�dy prvn� �loze OS. Tato �loha b�v� ozna�ov�na
jako "startovac� �loha" a v jej�m t�le jsou krom� statistick� �lohy a jej� samotn� funk�nosti
vytvo�eny tak� dal�� �lohy. Uk�zka, jak je vol�na, bude nast�n�na v popisu vytvo�en� uk�zkov� 
aplikace nad uC/OS-II.

Na tomto m�st� je nutn� podotknout, �e statistick� �loha je generov�na pouze v p��pad�,
�e je konstanta OS_TASK_STAT_EN nastavena na 1. D�le je nutn� zm�nit, �e na mikrokontroleru
MSP430F168, co� odpov�d� verzi FITKit 1.2, se nepoda�ilo statistickou �lohu zprovoznit.
Proto�e v�ak MSP430F168 obsahuje velice malou programovou pam� a nen� re�ln� zprovoznit
n�jakou v�razn� u�ite�nou a rozs�hlou aplikaci nad uC/OS-II, t�mto probl�mem se nebudeme 
d�le zab�vat.

Uk�zkov� aplikace nad uC/OS-II
================================
Proto�e se poda�ilo rozchodit uC/OS-II na platform�ch FITKit 1.2 i 2.0 (a� pro 1.2 
s jist�mi omezen�mi), jsou pro jednotliv� verze FITKitu aplikace rozd�ln�. K�d aplikace
se nach�z� v souboru ``app.c``. Proto�e p�ekladov� syst�m QDevKit neum� na z�klad�
verze FITKitu vybrat soubory, jsou ob� aplikace definov�ny v tomto souboru a direktivami
``#if`` je p�i p�ekladu vyb�r�na spr�vn� aplikace pro konkr�tn� platformu. To sice vede
k m�n� p�ehledn�mu k�du, zachov�me v�ak funk�nost obou dvou verz� platformy FITKit.

Uk�zka pro FITKit 1.2
======================
Tuto aplikaci implementovali Ing. Ji�� Novot��k a Ing. Tom� Novotn� v r�mci projektu
do p�edm�tu ROS. Tuto aplikaci jsme se tedy rozhodli ponechat pro verzi FITKitu 1.2, 
nebo�, jak jsme ji� zm�nili, jin�, slo�it�j�� aplikace vyvinout pro MSP430F168 nejde,
pro uk�zku funk�nosti uC/OS-II v�ak posta�uje.

Aplikace zpracov�v� dv� �lohy - jedna blik� zelenou diodou D5, druh� �ervenou. Ob�
diody blikaj� s r�znou frekvenc� a �asov�mi rozestupy.

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

Pro inicializaci hardwaru (p�edev��m clock system) je naps�na funkce ``my_initialize_hardware()``,
kter� je inspirovan� k�dem z knihovny ``libfitkit``.

Funkce ``main()`` programu vypad� pro tuto aplikaci n�sledovn�:

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
  
Z hlediska uC/OS-II jsou d�le�it� vol�n� ``OSInit()``, ``OSTaskCreate()`` a ``OSStart()``.
``OSInit()`` provede inicializaci opera�n�ho syst�mu a jeho z�kladn�ch �loh. Vol�n�m 
``OSTaskCreate()`` jsou do syst�mu p�id�ny �lohy blik�n� zelen� a �erven� diody. 
``OSStart()`` pak provede spu�t�n� multitaskingu. Je d�le�it�, aby p�ed spu�t�n�m
multitaskingu byla povolena obecn� p�eru�en� a spu�t�n Watchdog �asova�! Pokud tomu
tak nebude, pak uC/OS-II nebude fungovat spr�vn�.

Uk�zka pro FITKit 2.0
======================
Pro vytvo�en� uk�zkov� aplikace na FITKit 2.0 jsme se rozhodli vyu��t knihovny
``libfitkit``, co� na FITKitu 1.2 nebylo mo�n� z d�vodu mal� kapacity MSP430F168.
Uk�zkov� aplikace tedy bude schopna zas�lat zpr�vy na termin�l, reagovat na stisk
ur�it�ch kl�ves, ovl�dat sv�cen� diod D5 a D6 a ovl�dat v�pis na displeji. Nav�c,
pokud je v souboru ``os_cfg.h`` nastavena konstanta OS_TASK_STAT_EN na 1, bude
spu�t�na statistick� funkce a po stisku ur�it� kl�vesy budou �daje o zat�en�
mikroprocesoru vypisov�ny na displej/termin�l. P�i vytv��en� aplikace jsme
se ��ste�n� inspirovali u uk�zkov� aplikace nad FreeRTOS.

Stav, v jak�m se aplikace nach�z�, budeme ukl�dat do glob�ln�ch prom�nn�ch:

::
  
  volatile char last_ch;
  unsigned char c_pressed = 0;            // priznak stisku klavesy 'C'
  unsigned char d_pressed = 0;            // priznak stisku klavesy 'D'
  unsigned char load_pressed = 0;         // priznak stisku klavesy '*'
  unsigned char previous_c_pressed = 0;   // priznak zpracovaneho stisku klavesy 'C'
  
  unsigned char light_red = 1;            // priznak stisku klavesy 'A' - dioda D6
  unsigned char light_green = 1;          // priznak stisku klavesy 'B' - dioda D5

Prvn� �lohou, kterou bude OS zpracov�vat, bude pr�ce s kl�vesnic�.  Ta bude v p��pad�
stisku kl�vesy 'A' rozsv�cet/zhas�nat �ervenou diodu D6 a odes�lat o tom zpr�vu na termin�l,
p�i stisku kl�vesy 'B' rozsv�cet/zhas�nat zelenou diodu D5 a odes�lat o tom zpr�vu na termin�l.
P�i stisku kl�vesy 'C' bude nastavov�n p��znak jej�ho stisknut� a bude o tom odesl�na
zpr�va na termin�l - kl�vesa 'C' bude ovl�dat zapnut�/vypnut� displeje.
P�i stisku kl�vesy 'D' bude vyb�r�no, zda na displej bude vypisov�n �daj
o rozsv�cen�ch/zhasnut�ch diod�ch nebo zda bude vypisov�na z�t� mikroprocesoru
vypo��tan� statistickou �lohou (pokud je generov�na). P�i stisku kl�vesy '*' bude
nastaven p��znak generov�n� z�t�e v r�mci z�t�ov� �lohy.

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

Druh� �loha bude m�t za �kol ovl�d�n� displeje na z�klad� p��znak� stisknut�ch kl�ves.
Na z�klad� stisku kl�ves/stavu diod bude na displej implicitn� vypisovat, v jak�m
stavu se diody nach�z� (nap�. D5:OFF D6:ON). V p��pad�, �e je nastaven p��znak
po stisknut� kl�vesy 'C', displej je pova�ov�n za vypnut� a nevypisuje se na n�j nic.
V p��pad�, �e je nastaven p��znak kl�vesou 'D', a nen� displej vypnut� kl�vesou 'C',
je na displej m�sto �daj� o diod�ch vypisov�n stav zat�en� mikroprocesoru (nap�. 
CPU 98%). V p��pad�, �e je nastaven p��znak kl�vesy '*', je ��zen� displeje
ponech�no na z�t�ov� �loze ``loadTask()``.

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

T�et� �loha, z�t�ov� �loha ``loadTask()``, je v aplikaci pro demonstraci
funk�nosti statistick� funkce. Tato �loha, pokud je nastaven p��slu�n� p��znak,
generuje z�t� mikro�ipu rychl�m obnovov�n�m textu na displeji (to je �in�no ka�d�ch
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


V aplikaci je je�t� definov�na t�et� �loha. Tato �loha je ozna�ena jako ``startTask()``
a je v n�, pokud je to vy�adov�no, inicializov�na statistick� funkce. V t�le t�to funkce
jsou tak� inicializov�ny ostatn� �lohy. Samotn� �loha jinak nic neprov�d�.

`Pozn.: Maximum �loh v aplikaci se nastavuje v souboru` ``os_cfg.h`` `konstantou`
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
  
Funkce ``main()`` cel� aplikace pak vypad� takto:

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
  
Inicializace HW je ponech�na v tomto p��pad� na funkci ``initialize_hardware()``
z knihovny ``libfitkit``. Na t�to funkci ``main()`` se ale od t� p�edchoz� pro 
FITKit 1.2 mnoho neli�� - jedin�m podstatn�m rozd�lem je vytvo�en� �loh OS a�
v r�mci �lohy ``startTask()``.

Zprovozn�n� aplikace
=====================
Pro spr�vn� chod aplikace je nutn� m�t adres�� se zdrojov�mi k�dy um�st�n� v 
svn adres��i FITKitu, v ``/apps/demo_msp``. Pro zprovozn�n� aplikace na FITKitu
sta�� p�elo�it aplikaci, naprogramovat MCU a FPGA a pot� spustit termin�lov� program.

Pod�kov�n�
============
T�mto bychom cht�li pod�kovat za pomoc a konzultace Ing. Ji��mu Novot��kovi a Ing.
Tom�i Novotn�mu, kte�� ji� d��ve s uC/OS-II pracovali a poskytli n�m k dispozici 
v�sledky jejich pr�ce, na kter� jsme mohli nav�zat a t�m tento projekt rozvinout.
Takt� d�kujeme Ing. Josefu Strnadelovi, Ph.D., za poskytnut� studijn� materi�ly.

Pou�it� literatura
===================
Labrosse J. Jean: uC/OS-II Real-Time Kernel

Texas Instruments: MSP430x1xx Family User's Guide

Texas Instruments: MSP430x2xx Family User's Guide
