;********************************************************************************************************
; Projekt do predmetu IMP
; uC/OS-II pro platformu FITKit 1.2 a 2.0
; Autori: Karel Koranda     xkoran01@stud.fit.vutbr.cz
;         Vladimir Bruzek   xbroze01@stud.fit.vutbr.cz
;         Tomas Jilek       xjilek02@stud.fit.vutbr.cz
; Datum: 26.11.2010
; 
; V kodu byla zavedena definice maker PUSHALL a POPALL pro zprehledneni kodu
; a kod byl upraven tak, aby byl prelozitelny systemem QDevKit a pro oba
; mikrokontrolery MSP430(F168 a F2617) na FITKitech 1.2 a 2.0 .
;
; Pro preklad v systemu QDevKit bez vypisu warningu byl zaveden k tomuto souboru
; hlavickovy soubor os_cpu_a.h .
;********************************************************************************************************
; RTOS uC/OS-II pro platformu FITkit
; Autori: Jiri Novotnak (xnovot87), Tomas Novotny (xnovot88)
; Brno, 12/2009
; Kod vznikl upravou kodu pro jiny mikrokontroler, originalni hlavicka:
;********************************************************************************************************
;                                               uC/OS-II
;                                         The Real-Time Kernel
;
;                              (c) Copyright 2008, Micrium, Inc., Weston, FL
;                                          All Rights Reserved
;
;                                               TI MSP430X
;                                               MSP430F2617
;
;
; File         : OS_CPU_A.S
; By           : Jian Chen (yenger@hotmail.com)
;                Jean J. Labrosse
;********************************************************************************************************

;********************************************************************************************************
; MACRO DEFINITIONS
;********************************************************************************************************

  .macro PUSHALL
          PUSH.W      R15
          PUSH.W      R14
          PUSH.W      R13
          PUSH.W      R12
          PUSH.W      R11
          PUSH.W      R10
          PUSH.W      R9
          PUSH.W      R8
          PUSH.W      R7
          PUSH.W      R6
          PUSH.W      R5
          PUSH.W      R4
  .endm
  
  .macro POPALL
          POP.W      R4
          POP.W      R5
          POP.W      R6
          POP.W      R7
          POP.W      R8
          POP.W      R9
          POP.W      R10
          POP.W      R11
          POP.W      R12
          POP.W      R13
          POP.W      R14
          POP.W      R15
  .endm
           
;********************************************************************************************************
;                                  PUBLIC AND EXTERNAL DECLARATIONS
;********************************************************************************************************

	.extern  OSIntExit
	.extern  OSIntNesting

	.extern  OSISRStkPtr

	.extern  OSPrioCur
	.extern  OSPrioHighRdy

	.extern  OSRunning

	.extern  OSTCBCur
	.extern  OSTCBHighRdy

	.extern  OSTaskSwHook
	.extern  OSTimeTick

	.global  OSCtxSw
	.global  OSCPURestoreSR
	.global  OSCPUSaveSR
	.global  OSIntCtxSw
	.global  OSStartHighRdy

;********************************************************************************************************
;                                  START HIGHEST PRIORITY READY TASK
;
; Description: This function is called by OSStart() to start the highest priority task that is ready to run.
;
; Note       : OSStartHighRdy() MUST:
;                 a) Call OSTaskSwHook() then,
;                 b) Set OSRunning to TRUE,
;                 c) Switch to the highest priority task.
;********************************************************************************************************

.text

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

;********************************************************************************************************
;                                     TASK LEVEL CONTEXT SWITCH
;
; Description: This function is called by OS_Sched() to perform a task level context switch.
;
; Note       : OSCtxSw() MUST:
;                 a) Save the current task's registers onto the current task stack
;                 b) Save the SP into the current task's OS_TCB
;                 c) Call OSTaskSwHook()
;                 d) Copy OSPrioHighRdy to OSPrioCur
;                 e) Copy OSTCBHighRdy to OSTCBCur
;                 f) Load the SP with OSTCBHighRdy->OSTCBStkPtr
;                 g) Restore all the registers from the high priority task stack
;                 h) Perform a return from interrupt
;********************************************************************************************************

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

;********************************************************************************************************
;                                       ISR LEVEL CONTEXT SWITCH
;
; Description: This function is called by OSIntExit() to perform an ISR level context switch.
;
; Note       : OSIntCtxSw() MUST:
;                 a) Call OSTaskSwHook()
;                 b) Copy OSPrioHighRdy to OSPrioCur
;                 c) Copy OSTCBHighRdy to OSTCBCur
;                 d) Load the SP with OSTCBHighRdy->OSTCBStkPtr
;                 e) Restore all the registers from the high priority task stack
;                 f) Perform a return from interrupt
;********************************************************************************************************

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

;********************************************************************************************************
;                                              TICK ISR
;
; Description: This ISR handles tick interrupts.  This ISR uses the Watchdog timer as the tick source.
;
; Notes      : 1) The following C pseudo-code describes the operations being performed in the code below.
;
;                 Save all the CPU registers
;                 if (OSIntNesting == 0) {
;                     OSTCBCur->OSTCBStkPtr = SP;
;                     SP                    = OSISRStkPtr;  /* Use the ISR stack from now on           */
;                 }
;                 OSIntNesting++;
;                 Enable interrupt nesting;                 /* Allow nesting of interrupts (if needed) */
;                 Clear the interrupt source;
;                 OSTimeTick();                             /* Call uC/OS-II's tick handler            */
;                 DISABLE general interrupts;               /* Must DI before calling OSIntExit()      */
;                 OSIntExit();
;                 if (OSIntNesting == 0) {
;                     SP = OSTCBHighRdy->OSTCBStkPtr;       /* Restore the current task's stack        */
;                 }
;                 Restore the CPU registers
;                 Return from interrupt.
;
;              2) ALL ISRs should be written like this!
;
;              3) You MUST disable general interrupts BEFORE you call OSIntExit() because an interrupt
;                 COULD occur just as OSIntExit() returns and thus, the new ISR would save the SP of
;                 the ISR stack and NOT the SP of the task stack.  This of course will most likely cause
;                 the code to crash.  By disabling interrupts BEFORE OSIntExit(), interrupts would be
;                 disabled when OSIntExit() would return.  This assumes that you are using OS_CRITICAL_METHOD
;                 #3 (which is the prefered method).
;
;              4) If you DON'T use a separate ISR stack then you don't need to disable general interrupts
;                 just before calling OSIntExit().  The pseudo-code for an ISR would thus look like this:
;
;                 Save all the CPU registers
;                 if (OSIntNesting == 0) {
;                     OSTCBCur->OSTCBStkPtr = SP;
;                 }
;                 OSIntNesting++;
;                 Enable interrupt nesting;                 /* Allow nesting of interrupts (if needed) */
;                 Clear the interrupt source;
;                 OSTimeTick();                             /* Call uC/OS-II's tick handler            */
;                 OSIntExit();
;                 Restore the CPU registers
;                 Return from interrupt.
;********************************************************************************************************

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

;********************************************************************************************************
;                             SAVE AND RESTORE THE CPU'S STATUS REGISTER
;
; Description: These functions are used to implement OS_CRITICAL_METHOD #3 by saving the status register
;              in a local variable of the calling function and then, disables interrupts.
;
; Notes      : R15 is assumed to hold the argument passed to OSCPUSaveSR() and also, the value returned
;              by OSCPURestoreSR().
;********************************************************************************************************

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
