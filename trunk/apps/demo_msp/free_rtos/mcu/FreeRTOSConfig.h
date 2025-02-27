/*
	FreeRTOS.org V5.0.0 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and 
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety 
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting, 
	licensing and training services.
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <fitkitlib.h>

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION      1                                 /* Set to 1 to use the preemptive kernel, or 0 to use the cooperative kernel */ 

#define configUSE_IDLE_HOOK	      0                                 /* Set to 1 if you wish to use an idle hook, or 0 to omit an idle hook */
#define configUSE_TICK_HOOK	      0                                 /* Set to 1 if you wish to use an tick hook, or 0 to omit an tick hook */

#define configUSE_16_BIT_TICKS		1                                 /* Time is measured in 'ticks' - which is the number of times the tick interrupt has executed since the kernel was started. The tick count is held in a variable of type portTickType */
#define configCPU_CLOCK_HZ        ( ( unsigned portLONG ) 7372800 ) /* Enter the frequency in Hz at which the internal processor core will be executing. This value is required in order to correctly configure timer peripherals */
#define configTICK_RATE_HZ        ( ( portTickType ) 200 )          /* The frequency of the RTOS tick interrupt */
                                                                    /* portTICK_RATE_MS defined as 1000/configTICK_RATE_HZ */

#define configMINIMAL_STACK_SIZE	( ( unsigned portSHORT ) 32 )     /* The size of the stack used by the idle task. Generally this should not be reduced from the value set in the FreeRTOSConfig.h file provided with the demo application for the port you are using */
#define configTOTAL_HEAP_SIZE		  ( ( size_t ) ( 1024 ) )           /* This value will only be used if your application makes use of one of the sample memory allocation schemes provided in the FreeRTOS source code download. See the memory configuration section for further details */

#define configMAX_PRIORITIES		  ( ( unsigned portBASE_TYPE ) 4 )  /* The number of priorities available to the application tasks. Any number of tasks can share the same priority. Co-routines are prioritised separately - see configMAX_CO_ROUTINE_PRIORITIES */
#define configMAX_TASK_NAME_LEN		( 8 )                             /* The maximum permissible length of the descriptive name given to a task when the task is created. The length is specified in the number of characters including the NULL termination byte */
#define configIDLE_SHOULD_YIELD		0                                 /* This parameter controls the behaviour of tasks at the idle priority. It only has an effect if: a) the preemptive scheduler is being used and b) the users application creates tasks that run at the idle priority */

#define configUSE_TRACE_FACILITY  0                                 /* Set to 1 if you wish the trace visualisation functionality to be available, or 0 if the trace functionality is not going to be used. If you use the trace functionality a trace buffer must also be provided */

#define configUSE_MUTEXES              0                            /* Set to 1 to include mutex functionality in the build, or 0 to omit mutex functionality from the build. Readers should familiarise themselves with the differences between mutexes and binary semaphores in relation to the FreeRTOS.org functionality */
#define configUSE_RECURSIVE_MUTEXES    0                            /* Set to 1 to include recursive mutex functionality in the build, or 0 to omit recursive mutex functionality from the build */
#define configUSE_COUNTING_SEMAPHORES  0                            /* Set to 1 to include counting semaphore functionality in the build, or 0 to omit counting semaphore functionality from the build */
#define configUSE_ALTERNATIVE_API      0                            /* Set to 1 to include the 'alternative' queue functions in the build, or 0 to omit the 'alternative' queue functions from the build. The alternative API is described within the queue.h header file */
#define configCHECK_FOR_STACK_OVERFLOW 0                            /* The stack overflow detection page describes the use of this parameter */
#define configQUEUE_REGISTRY_SIZE      0                           /* Defines the maximum number of queues and semaphores that can be registered. Only those queues and semaphores that you want to view using a kernel aware debugger need be registered. See the API reference documentation for vQueueAddToRegistry() and vQueueUnregisterQueue() for more information. The queue registry has no purpose unless you are using a kernel aware debugger */

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		      0                           /* Set to 1 to include co-routine functionality in the build, or 0 to omit co-routine functionality from the build. To include co-routines croutine.c must be included in the project */
#define configMAX_CO_ROUTINE_PRIORITIES ( 0 )                       /* The number of priorities available to the application co-routines. Any number of co-routines can share the same priority. Tasks are prioritised separately - see configMAX_PRIORITIES */

/* Set the following definitions to 1 to include the API function, or zero to exclude the API function. */
#define INCLUDE_vTaskPrioritySet      0                             
#define INCLUDE_uxTaskPriorityGet		  0                             
#define INCLUDE_vTaskDelete				    0                             
#define INCLUDE_vTaskCleanUpResources	0                             
#define INCLUDE_vTaskSuspend			    0                             
#define INCLUDE_vTaskDelayUntil			  0                             
#define INCLUDE_vTaskDelay				    1                             

/*
#define TASK_PRINTF(pxNextTCB,cStatus,usStackRemaining) { \
      term_send_str(" TASK "); \
      term_send_num(pxNextTCB->uxTCBNumber); \
      term_send_char(' '); \
      term_send_str((char *)pxNextTCB->pcTaskName); \
      term_send_char(' '); \
      term_send_char(cStatus); \
      term_send_char(' '); \
      term_send_num(pxNextTCB->uxPriority); \
      term_send_char(' '); \
      term_send_num(pxNextTCB->uxStackDepth); \
      term_send_char('/'); \
      term_send_num(usStackRemaining); \
      term_send_crlf(); \
   }
*/

#endif /* FREERTOS_CONFIG_H */
